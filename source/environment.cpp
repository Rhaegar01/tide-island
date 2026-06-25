#include "environment.hpp"
#include "log.hpp"

#include <array>
#include <cstdlib>
#include <expected>
#include <filesystem>
#include <format>
#include <mutex>
#include <ranges>
#include <string>
#include <string_view>

#include <unistd.h>
#include <GLES3/gl3.h>

using namespace std;
namespace fs = filesystem;

namespace {

enum class RendererKind {
    Hardware,
    Llvmpipe,
    Softpipe,
    OtherSoftware,
};

[[nodiscard]] string_view env_value(const char* name) noexcept {
    if (const char* v = ::getenv(name))
        return v;
    return {};
}

[[nodiscard]] bool env_is_set(const char* name) noexcept {
    return ::getenv(name) != nullptr;
}

[[nodiscard]] bool env_is_true(const char* name) noexcept {
    const auto v = env_value(name);
    return !v.empty() && v != "0" && v != "false" && v != "FALSE";
}

constexpr array software_renderer_names = {
    string_view{"llvmpipe"},
    string_view{"softpipe"},
    string_view{"swrast"},
};

[[nodiscard]] RendererKind classify_renderer(string_view renderer) noexcept {
    if (renderer.contains("llvmpipe")) return RendererKind::Llvmpipe;
    if (renderer.contains("softpipe")) return RendererKind::Softpipe;
    if (renderer.contains("swrast"))   return RendererKind::OtherSoftware;
    return RendererKind::Hardware;
}

[[nodiscard]] bool explicit_driver_selected() noexcept {
    return env_is_set("GALLIUM_DRIVER")            ||
           env_is_set("LIBGL_ALWAYS_SOFTWARE")     ||
           env_is_set("MESA_LOADER_DRIVER_OVERRIDE");
}

[[nodiscard]] bool explicit_software_requested() noexcept {
    if (env_is_true("LIBGL_ALWAYS_SOFTWARE")) return true;

    // 使用 ranges 版本，完美支持直接传入 {"..."}
    return
        ranges::contains(software_renderer_names, env_value("GALLIUM_DRIVER")) ||
        ranges::contains(software_renderer_names, env_value("MESA_LOADER_DRIVER_OVERRIDE"));
}

[[nodiscard]] bool render_node_accessible() noexcept {
    error_code ec;
    if (!fs::exists("/dev/dri", ec) || ec) return false;

    const fs::directory_iterator it("/dev/dri", ec);
    if (ec) return false;

    return ranges::any_of(it, [](const fs::directory_entry& entry) {
        const string filename = entry.path().filename().string();
        return filename.starts_with("renderD") &&
               ::access(entry.path().c_str(), R_OK | W_OK) == 0;
    });
}

[[nodiscard]] expected<string_view, string_view>
query_gl_string(GLenum name) noexcept {
    const auto* p = reinterpret_cast<const char*>(::glGetString(name));
    if (!p) return unexpected{"<unavailable>"};
    return string_view{p};
}

[[nodiscard]] string join_lines(initializer_list<string_view> lines) {
    if (lines.size() == 0) return {};

    const size_t total = ranges::fold_left(lines, size_t{0},
        [](size_t acc, string_view sv) { return acc + sv.size() + 1; }) - 1;

    string msg;
    msg.reserve(total);

    bool first = true;
    for (const string_view line : lines) {
        if (!first) msg.push_back('\n');
        msg.append(line);
        first = false;
    }
    return msg;
}

once_flag g_log_once_flag;

void log_once(Log::LogLevel level, initializer_list<string_view> lines) {
    call_once(g_log_once_flag, [&] {
        frame_logger(level, join_lines(lines));
    });
}

void set_softpipe_for_next_context() {
    ::setenv("GALLIUM_DRIVER", "softpipe", 1);
    ::unsetenv("LIBGL_ALWAYS_SOFTWARE");
}

} // namespace

namespace GraphicBackend {

void prepare_graphics_backend() {
    if (explicit_driver_selected())  return;
    if (render_node_accessible())    return;

    set_softpipe_for_next_context();

    log_once(Log::Warning, {
        "We noticed that Tide-island is being rendered on the CPU because no usable GPU backend is currently available.",
        "",
        "This uses softpipe, which increases CPU usage.",
        "",
        "If you want Tide-island to use GPU rendering, please check that the GPU driver is installed "
        "and that the application can access /dev/dri/renderD*."
    });
}

GraphicBackendDecision inspect_graphics_backend_after_context() {
    const auto renderer_result = query_gl_string(GL_RENDERER);
    const auto vendor_result   = query_gl_string(GL_VENDOR);
    const auto version_result  = query_gl_string(GL_VERSION);

    if (!renderer_result) {
        log_once(Log::Warning, {
            "We could not query the current graphics renderer.",
            "",
            "The graphics backend check is incomplete."
        });
        return GraphicBackendDecision::KeepCurrentContext;
    }

    const string_view renderer = *renderer_result;

    const RendererKind kind = classify_renderer(renderer);

    if (explicit_software_requested()) {
        switch (kind) {
        case RendererKind::Llvmpipe:
            log_once(Log::Warning, {
                "We noticed that you have set the environment to llvmpipe, "
                "so Tide-island will be rendered on the CPU instead of the GPU.",
                "",
                "This can increase Tide-island's memory usage by more than 80 MB "
                "because llvmpipe loads LLVM for software rendering.",
                "",
                "If you want Tide-island to use GPU rendering, please unset GALLIUM_DRIVER "
                "or change it to a hardware driver."
            });
            break;

        case RendererKind::Softpipe:
            log_once(Log::Warning, {
                "We noticed that you have set the environment to softpipe, "
                "so Tide-island will be rendered on the CPU instead of the GPU.",
                "",
                "This increases CPU usage.",
                "",
                "If you want Tide-island to use GPU rendering, please unset GALLIUM_DRIVER "
                "or change it to a hardware driver."
            });
            break;

        default:
            log_once(Log::Warning, {
                "We noticed that software rendering was explicitly requested by the environment.",
                "",
                "Tide-island will respect this configuration.",
                "",
                format("Vendor: {}",  vendor_result.value_or("unknown")),
                format("Version: {}", version_result.value_or("unknown")),
            });
            break;
        }
        return GraphicBackendDecision::KeepCurrentContext;
    }

    if (kind == RendererKind::Hardware) {
        Log::logger(Log::Debug, "Using GPU renderer: {}", renderer);
        return GraphicBackendDecision::KeepCurrentContext;
    }

    switch (kind) {
    case RendererKind::Llvmpipe:
        log_once(Log::Warning, {
            "We noticed that Tide-island is being rendered on the CPU because "
            "no usable GPU backend is currently available.",
            "",
            "This is using llvmpipe, which can increase Tide-island's memory usage "
            "by more than 80 MB because LLVM is loaded for software rendering.",
            "",
            "If you want Tide-island to use GPU rendering, please make sure the GPU driver "
            "is installed and that the application can access /dev/dri/renderD*.",
            "",
            "Possible causes:",
            "- GPU driver is missing or broken",
            "- /dev/dri/renderD* is not accessible",
            "- running inside a container or remote session",
            "- software rendering was forced by the environment"
        });
        set_softpipe_for_next_context();
        return GraphicBackendDecision::RecreateContextWithSoftpipe;

    case RendererKind::Softpipe:
        log_once(Log::Warning, {
            "We noticed that Tide-island is being rendered on the CPU because "
            "no usable GPU backend is currently available.",
            "",
            "This uses softpipe, which increases CPU usage.",
            "",
            "If you want Tide-island to use GPU rendering, please make sure the GPU driver "
            "is installed and that the application can access /dev/dri/renderD*."
        });
        return GraphicBackendDecision::KeepCurrentContext;

    default:
        log_once(Log::Warning, {
            "We noticed that Tide-island is not using a GPU renderer.",
            "",
            "Please check whether the GPU driver is installed and whether "
            "the application can access /dev/dri/renderD*."
        });
        return GraphicBackendDecision::KeepCurrentContext;
    }
}

} // namespace GraphicBackend

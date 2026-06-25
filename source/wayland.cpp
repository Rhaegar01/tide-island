// ============================================================================
// Tide Island Wayland backend
// ============================================================================
//
// This translation unit owns the native Wayland, layer-shell, EGL window, and
// swapchain-facing platform state used by the renderer.
//
#include "wayland.hpp"
#include "renderer.hpp"
#include "island.hpp"
#include "log.hpp"

#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-egl-core.h>
#include <wayland-egl.h>
#include <algorithm>
#include <array>
#include <memory>
#include <print>
#include <expected>
#include <string_view>

using namespace std;

// ============================================================================
// [Internal Details]
// ============================================================================

namespace {

template <auto delete_func>
struct DeleteWayland {
    void operator()(auto* ptr) const noexcept {
        if (ptr) {
            delete_func(ptr);
        }
    }
};

struct Context {
    // Wayland Objects
    unique_ptr<wl_display, DeleteWayland<wl_display_disconnect>> display{
        nullptr
    };
    unique_ptr<wl_registry, DeleteWayland<wl_registry_destroy>> registry{
        nullptr
    };
    unique_ptr<wl_compositor, DeleteWayland<wl_compositor_destroy>> compositor{
        nullptr
    };
    unique_ptr<zwlr_layer_shell_v1, DeleteWayland<zwlr_layer_shell_v1_destroy>>
        layer_shell{nullptr};
    unique_ptr<wl_surface, DeleteWayland<wl_surface_destroy>> surface{
        nullptr
    };
    unique_ptr<zwlr_layer_surface_v1, DeleteWayland<zwlr_layer_surface_v1_destroy>>
        layer_surface{nullptr};
    unique_ptr<wl_egl_window, DeleteWayland<wl_egl_window_destroy>> egl_window{
        nullptr
    };

    // EGL Objects
    EGLDisplay egl_display{EGL_NO_DISPLAY};
    EGLConfig  egl_config{nullptr};
    EGLContext egl_context{EGL_NO_CONTEXT};
    EGLSurface egl_surface{EGL_NO_SURFACE};
};

Context context;

// --- Wayland Registry Listeners ---

void registry_global(
    void*,
    wl_registry* registry,
    uint32_t name,
    const char* interface,
    uint32_t version
) {
    if (string_view(interface) == wl_compositor_interface.name) {
        context.compositor.reset(static_cast<wl_compositor*>(
            wl_registry_bind(registry, name, &wl_compositor_interface, min(version, 4u))
        ));
    }
    else if (string_view(interface) == zwlr_layer_shell_v1_interface.name) {
        context.layer_shell.reset(static_cast<zwlr_layer_shell_v1*>(
            wl_registry_bind(registry, name, &zwlr_layer_shell_v1_interface, min(version, 4u))
        ));
    }
}

void registry_remove(void*, wl_registry*, uint32_t id) {
    logger(Log::Error, "[Info] Wayland global resource removed (id: {})", id);
}

constexpr wl_registry_listener registry_listener = {
    .global        = registry_global,
    .global_remove = registry_remove,
};

// --- Layer Surface Listeners ---

void layer_surface_configure(
    void*,
    zwlr_layer_surface_v1* surface,
    uint32_t serial,
    uint32_t width,
    uint32_t height
) {
    zwlr_layer_surface_v1_ack_configure(surface, serial);
    logger(Log::Debug, "Configured: {} {}", width, height);
    Island::set_window_size(static_cast<int>(width), static_cast<int>(height));
}

void layer_surface_closed(void*, zwlr_layer_surface_v1*) {
    logger(Log::Error, "[Warn] Layer surface was closed by compositor. Exiting...");
    Island::set_is_running(false);
}

constexpr zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = layer_surface_configure,
    .closed    = layer_surface_closed,
};

} // namespace

// ============================================================================
// [Public API Implementation]
// ============================================================================

expected<void, const char*> Wayland::init() {
    Island::Island island_state = Island::state();

    if (island_state.window_width == 0 || island_state.window_height == 0) {
        return unexpected("Size of island is 0, call 'Island::set_window_size(int w, int h)'");
    }

    // 1. Establish Wayland Connection & Registry
    context.display.reset(wl_display_connect(nullptr));
    if (!context.display) {
        return unexpected("Failed to connect Wayland");
    }

    context.registry.reset(wl_display_get_registry(context.display.get()));
    
    if (!context.registry.get()){
        return unexpected("Failed to get registry");
    }

    wl_registry_add_listener(context.registry.get(), &registry_listener, nullptr);
    wl_display_roundtrip(context.display.get());

    if (wl_display_roundtrip(context.display.get()) == -1) {
        return unexpected("roundtrip failed");
    }

    if (!context.compositor) {
        return unexpected("No compositor found");
    }

    if (!context.layer_shell) {
        return unexpected("No layer shell found");
    }

    // 2. Setup Wayland Surface & Layer Shell
    context.surface.reset(wl_compositor_create_surface(context.compositor.get()));

    if (!context.surface.get()){
        return unexpected("Failed to sest surface");
    }

    context.layer_surface.reset(zwlr_layer_shell_v1_get_layer_surface(
        context.layer_shell.get(),
        context.surface.get(),
        nullptr,
        ZWLR_LAYER_SHELL_V1_LAYER_TOP,
        "tide-island"
    ));

    if (!context.layer_surface) {
        return unexpected("Failed to create layer surface");
    }

    zwlr_layer_surface_v1_set_anchor(
        context.layer_surface.get(),
        ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP
    );
    Wayland::request_resize(island_state.window_width, island_state.window_height);
    zwlr_layer_surface_v1_set_exclusive_zone(context.layer_surface.get(), island_state.zone);

    zwlr_layer_surface_v1_add_listener(
        context.layer_surface.get(),
        &layer_surface_listener,
        nullptr
    );
    wl_surface_commit(context.surface.get());

    if (wl_display_roundtrip(context.display.get()) == -1) {
        return unexpected("Wayland roundtrip failed");
    }

    // 3. Setup EGL Window & Display
    context.egl_window.reset(wl_egl_window_create(
        context.surface.get(),
        island_state.window_width,
        island_state.window_height
    ));
    if (!context.egl_window) {
        return unexpected("Failed to create wl_egl_window");
    }

    context.egl_display = eglGetDisplay((EGLNativeDisplayType)context.display.get());
    if (context.egl_display == EGL_NO_DISPLAY) {
        return unexpected("eglGetDisplay failed");
    }

    EGLint major{};
    EGLint minor{};
    if (!eglInitialize(context.egl_display, &major, &minor)) {
        return unexpected("eglInitialize failed");
    }
    logger(Log::Debug, "Using EGL {}.{}", major, minor);

    if (!eglBindAPI(EGL_OPENGL_ES_API)) {
        return unexpected("eglBindAPI failed");
    }
    
    // 4. Configure EGL Surface & Context
    constexpr array<EGLint, 13> attribs = {
        EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
        EGL_RED_SIZE,        8,
        EGL_GREEN_SIZE,      8,
        EGL_BLUE_SIZE,       8,
        EGL_ALPHA_SIZE,      8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_NONE
    };

    EGLint count{};
    if (!eglChooseConfig(context.egl_display, attribs.data(), &context.egl_config, 1, &count)
        || count == 0) {
        return unexpected("eglChooseConfig failed");
    }

    constexpr array<EGLint, 3> ctx_attribs = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };

    context.egl_context = eglCreateContext(
        context.egl_display,
        context.egl_config,
        EGL_NO_CONTEXT,
        ctx_attribs.data()
    );
    if (context.egl_context == EGL_NO_CONTEXT) {
        return unexpected("eglCreateContext failed");
    }

    context.egl_surface = eglCreateWindowSurface(
        context.egl_display,
        context.egl_config,
        (EGLNativeWindowType)context.egl_window.get(),
        nullptr
    );

    if (context.egl_surface == EGL_NO_SURFACE) {
        return unexpected("eglCreateWindowSurface failed");
    }

    if (!eglMakeCurrent(
        context.egl_display,
        context.egl_surface,
        context.egl_surface,
        context.egl_context
    )) {
        return unexpected("eglMakeCurrent failed");
    }

    return {};
}

void Wayland::dispatch_events() {
    if (wl_display_dispatch(context.display.get()) == -1)
        logger(Log::Error, "Failed to dispatch event");
}

void Wayland::swap_buffer() {
    eglSwapBuffers(context.egl_display, context.egl_surface);
}

void Wayland::request_resize(int width, int height) {
    if (!context.layer_surface) {
        logger(Log::Error, "request_resize called before layer_surface creation");
        return;
    }
    zwlr_layer_surface_v1_set_size(context.layer_surface.get(), width, height);
    Island::set_window_size(width, height);
}

void Wayland::shutdown() {
    if (!context.display) return;

    // 1. Terminate EGL Environment
    if (context.egl_display != EGL_NO_DISPLAY) {
        eglMakeCurrent(
            context.egl_display,
            EGL_NO_SURFACE,
            EGL_NO_SURFACE,
            EGL_NO_CONTEXT
        );

        if (context.egl_surface != EGL_NO_SURFACE) {
            eglDestroySurface(context.egl_display, context.egl_surface);
            context.egl_surface = EGL_NO_SURFACE;
        }

        if (context.egl_context != EGL_NO_CONTEXT) {
            eglDestroyContext(context.egl_display, context.egl_context);
            context.egl_context = EGL_NO_CONTEXT;
        }

        eglTerminate(context.egl_display);
        context.egl_display = EGL_NO_DISPLAY;
        context.egl_config  = nullptr;
    }
}

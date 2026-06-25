#define SOKOL_IMPL
#include "island.hpp"
#include "renderer.hpp"
#include "environment.hpp"
#include "log.hpp"

#include "include/sokol_gfx.h"
#include "include/sokol_log.h"
#include "include/wayland.hpp"

#include <print>
#include <unistd.h>

using namespace std;

int main(){
    println("");

    frame_logger(Log::Warning,
        "This build was compiled in debug mode.",
        "Performance may be reduced and additional debug output may appear."
    );

    GraphicBackend::prepare_graphics_backend();

    // Initialize island before wayland

    Island::set_window_size(140, 40);
    Island::set_island_size(140, 38);
    Island::set_anchor_top(2);
    Island::set_radius(19);
    Island::set_zone(40);

    expected<void, const char*> wayland_status = Wayland::init(); 

    if (!wayland_status.has_value()) {
        logger(Log::Error, wayland_status.error());
        return 1;
    }

    logger(Log::Debug, "Initialize Wayland successfully");

    expected<void, const char*> renderer_status = Renderer::init(); 

    if (!renderer_status.has_value()) {
        logger(Log::Error, renderer_status.error());
        return 1;
    }
    
    logger(Log::Debug, "Initialize render successfully");

    GraphicBackend::inspect_graphics_backend_after_context();

    Renderer::frame();

    Wayland::swap_buffer();

    while (Island::state().is_running) {
        Wayland::dispatch_events();
    }

    Wayland::shutdown();
    Renderer::shutdown();
}

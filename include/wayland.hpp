#pragma once

#include <expected>

namespace Wayland {

std::expected<void, const char*> init();
void request_resize(int width, int height);
void swap_buffer();
void dispatch_events();
void shutdown();

}

#pragma once

#include <expected>

namespace Renderer {

std::expected<void, const char*> init();
void frame();
void shutdown();
}

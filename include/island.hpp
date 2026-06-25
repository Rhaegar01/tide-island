#pragma once

#include <cstdint>
#include <array>

namespace Island {

struct Island {
    std::array<float,4> color = {0,0,0,1};
    int window_width = 0;
    int window_height = 0;
    int zone = -1;
    float island_width = 0;
    float island_height = 0;
    float anchor_top = 0;
    float radius = 0;
    bool is_running = true;
    bool geometry_dirty = true;
};

const Island& state();
void set_anchor_top(float distance);
void set_island_size(float width, float height);
void set_window_size(int width, int height);
void set_is_running(bool state);
void set_radius(float radius);
void set_zone(int zone);
}

// ============================================================================
// Tide Island renderer backend
// ============================================================================
//
// This translation unit owns the Sokol graphics objects, per-frame draw setup,
// and shader uniform updates for the island surface.
//
#include "renderer.hpp"
#include "island.hpp"
#include "log.hpp"

#include "sokol_gfx.h"
#include "sokol_log.h"
#include "island.glsl.h"

#include <array>

using namespace std;

// ============================================================================
// [Internal Details]
// ============================================================================

namespace {

// --- Vertex Layout ---

struct Vertex {
    float x, y;
    float r, g, b, a;
};


array<Vertex, 4> vertexes;

// --- Renderer State ---

const Island::Island& island_state = Island::state();
sg_shader shader;
sg_pipeline pipeline;
sg_buffer buffer;
project_uniform_t vertex_uniform;
radius_uniform_t radius_uniform;


// --- Sokol Frame Descriptors ---

sg_swapchain make_swapchain() {
    sg_swapchain sc{};
    sc.width          = island_state.window_width;
    sc.height         = island_state.window_height;
    sc.sample_count   = 1;
    sc.color_format   = SG_PIXELFORMAT_RGBA8;
    sc.depth_format   = SG_PIXELFORMAT_NONE;
    sc.gl.framebuffer = 0;
    return sc;
}

// --- vertex_uniform ---

void update_vertex_uniform() {

    if (island_state.window_width == 0 || island_state.window_height == 0){
        logger(Log::Error, "Set size of island window first");
        return;
    }

    vertex_uniform.proj[0]  =  2.0f / island_state.window_width;
    vertex_uniform.proj[5]  = -2.0f / island_state.window_height;
    vertex_uniform.proj[10] =  1.0f;
    vertex_uniform.proj[12] = -1.0f;
    vertex_uniform.proj[13] =  1.0f;
    vertex_uniform.proj[15] =  1.0f;
}

void update_radius_uniform() {

    if (island_state.island_width == 0 || island_state.island_height == 0){
        logger(Log::Error, "Set size of island window first");
        return;
    }

    radius_uniform.half_size[0] = island_state.island_width / 2;
    radius_uniform.half_size[1] = island_state.island_height / 2;
    radius_uniform.radius = island_state.radius;
    radius_uniform.top_anchor = island_state.anchor_top;
}

// --- Geometry Generation ---

void update_vertexes() {
    vertexes = {{
        {
            0,
            island_state.anchor_top,
            island_state.color[0],
            island_state.color[1],
            island_state.color[2],
            island_state.color[3]
        },
        {
            island_state.island_width,
            island_state.anchor_top,
            island_state.color[0],
            island_state.color[1],
            island_state.color[2],
            island_state.color[3]
        },
        {
            0,
            island_state.anchor_top + island_state.island_height,
            island_state.color[0],
            island_state.color[1],
            island_state.color[2],
            island_state.color[3]
        },
        {
            island_state.island_width,
            island_state.anchor_top + island_state.island_height,
            island_state.color[0],
            island_state.color[1],
            island_state.color[2],
            island_state.color[3]
        }
    }};
}

} // namespace

// ============================================================================
// [Public API Implementation]
// ============================================================================

expected<void, const char*> Renderer::init() {

    sg_desc sgdesc{};
    sgdesc.logger.func = slog_func;
    sgdesc.environment.defaults.color_format = SG_PIXELFORMAT_RGBA8;
    sgdesc.environment.defaults.depth_format = SG_PIXELFORMAT_NONE;
    sgdesc.environment.defaults.sample_count = 1;
    
    sg_setup(&sgdesc);
    if (!sg_isvalid()) {
        unexpected("sg_setup failed");
    }

    shader = sg_make_shader(island_shader_desc(sg_query_backend()));
    if (sg_query_shader_state(shader) != SG_RESOURCESTATE_VALID) {
        unexpected("sg_make_shader failed");
        sg_shutdown();
    }

    sg_pipeline_desc pipeline_desc{};
    pipeline_desc.shader = shader;
    pipeline_desc.layout.attrs[ATTR_island_position].buffer_index = 0;
    pipeline_desc.layout.attrs[ATTR_island_position].format = SG_VERTEXFORMAT_FLOAT2;
    pipeline_desc.layout.attrs[ATTR_island_color].buffer_index = 0;
    pipeline_desc.layout.attrs[ATTR_island_color].format = SG_VERTEXFORMAT_FLOAT4;
    pipeline_desc.primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP;
    pipeline_desc.colors[0].blend.enabled = true;
    pipeline_desc.colors[0].blend.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
    pipeline_desc.colors[0].blend.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    pipeline_desc.colors[0].blend.op_rgb = SG_BLENDOP_ADD;
    pipeline_desc.colors[0].blend.src_factor_alpha = SG_BLENDFACTOR_ONE;
    pipeline_desc.colors[0].blend.dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    pipeline_desc.colors[0].blend.op_alpha = SG_BLENDOP_ADD;

    pipeline = sg_make_pipeline(&pipeline_desc);
    if (sg_query_pipeline_state(pipeline) != SG_RESOURCESTATE_VALID) {
        unexpected("sg_make_pipeline failed");
        sg_destroy_shader(shader);
        sg_shutdown();
    }

    sg_buffer_desc buffer_desc{};
    buffer_desc.size = sizeof(Vertex) * 4;
    buffer_desc.usage.dynamic_update = true;
    
    buffer = sg_make_buffer(&buffer_desc);
    if (sg_query_buffer_state(buffer) != SG_RESOURCESTATE_VALID) {
        unexpected("sg_make_buffer failed");
        sg_destroy_pipeline(pipeline);
        sg_destroy_shader(shader);
        sg_shutdown();
    }

    update_vertex_uniform();
    update_radius_uniform();
    
    return {};
}

void Renderer::frame() {
    sg_pass_action pass_action{};
    pass_action.colors[0].load_action = SG_LOADACTION_CLEAR;
    pass_action.colors[0].clear_value = {0.0f, 0.0f, 0.0f, 0.0f};

    sg_pass pass{};
    pass.action = pass_action;
    pass.swapchain = make_swapchain();
    sg_begin_pass(&pass);
    
    update_vertexes();

    sg_update_buffer(buffer, SG_RANGE(vertexes));
    sg_apply_pipeline(pipeline);
    sg_bindings bindings{};
    bindings.vertex_buffers[0] = buffer;
    sg_apply_bindings(&bindings);

    sg_apply_uniforms(UB_project_uniform, SG_RANGE(vertex_uniform));
    sg_apply_uniforms(UB_radius_uniform , SG_RANGE(radius_uniform));
    sg_draw(0, 4, 1);

    sg_end_pass();
    sg_commit();
}

void Renderer::shutdown() {
    sg_destroy_buffer(buffer);
    sg_destroy_pipeline(pipeline);
    sg_destroy_shader(shader);
    sg_shutdown();
    logger(Log::Debug, "Sokol graphics context destroyed.");
}

#pragma once

namespace GraphicBackend {

enum GraphicBackendDecision {
    KeepCurrentContext,
    RecreateContextWithSoftpipe
};

void prepare_graphics_backend();
GraphicBackendDecision inspect_graphics_backend_after_context();

}

#pragma once

namespace ToyRenderer {

struct ApiRenderPassCommandRecorder {
    virtual void end() = 0;
};

} // namespace ToyRenderer

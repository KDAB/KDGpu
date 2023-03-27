#pragma once

namespace KDGpu {

struct ApiCommandBuffer {
    virtual void begin() = 0;
    virtual void finish() = 0;
};

} // namespace KDGpu

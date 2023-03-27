#pragma once

namespace KDGpu {

struct ApiBuffer {
    virtual void *map() = 0;
    virtual void unmap() = 0;
};

} // namespace KDGpu

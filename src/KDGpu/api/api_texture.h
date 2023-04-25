#pragma once

namespace KDGpu {

struct ApiTexture {
    virtual void *map() = 0;
    virtual void unmap() = 0;
};

} // namespace KDGpu

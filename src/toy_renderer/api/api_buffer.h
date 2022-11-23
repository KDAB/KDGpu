#pragma once

namespace ToyRenderer {

struct ApiBuffer {
    virtual void *map() = 0;
    virtual void unmap() = 0;
};

} // namespace ToyRenderer

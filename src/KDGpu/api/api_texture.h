#pragma once

#include <KDGpu/texture.h>

namespace KDGpu {

struct ApiTexture {
    virtual void *map() = 0;
    virtual void unmap() = 0;
    virtual SubresourceLayout getSubresourceLayout(const TextureSubresource &subresource) const = 0;
};

} // namespace KDGpu

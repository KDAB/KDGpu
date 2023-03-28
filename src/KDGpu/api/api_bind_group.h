#pragma once

namespace KDGpu {

struct BindGroupEntry;

struct ApiBindGroup {
    virtual void update(const BindGroupEntry &entry) = 0;
};

} // namespace KDGpu

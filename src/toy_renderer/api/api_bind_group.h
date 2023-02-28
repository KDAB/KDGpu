#pragma once

namespace ToyRenderer {

struct BindGroupEntry;

struct ApiBindGroup {
    virtual void update(const BindGroupEntry &entry) = 0;
};

} // namespace ToyRenderer

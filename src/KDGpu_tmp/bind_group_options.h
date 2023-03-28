#pragma once

#include <kdgpu/bind_group_description.h>
#include <kdgpu/bind_group_layout.h>

namespace KDGpu {

struct BindGroupEntry { // An entry into a BindGroup ( == a descriptor in a descriptor set)
    uint32_t binding;
    BindingResource resource;
};

struct BindGroupOptions {
    Handle<BindGroupLayout_t> layout;
    std::vector<BindGroupEntry> resources;
};

} // namespace KDGpu

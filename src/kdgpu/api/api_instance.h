#pragma once

#include <kdgpu/gpu_core.h>
#include <kdgpu/handle.h>
#include <kdgpu/surface_options.h>

#include <vector>

namespace KDGpu {

struct Adapter_t;
struct Instance_t;
struct Surface_t;

struct ApiInstance {
    virtual std::vector<Extension> extensions() const = 0;
    virtual std::vector<Handle<Adapter_t>> queryAdapters(const Handle<Instance_t> &instanceHandle) = 0;
    virtual Handle<Surface_t> createSurface(const SurfaceOptions &options) = 0;
};

} // namespace KDGpu

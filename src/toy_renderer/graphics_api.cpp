#include "graphics_api.h"

namespace ToyRenderer {

GraphicsApi::GraphicsApi()
{
}

GraphicsApi::~GraphicsApi()
{
}

Instance GraphicsApi::createInstance(const InstanceOptions &options)
{
    auto instance = Instance(this, options);
    return std::move(instance);
}

} // namespace ToyRenderer

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
    return Instance(this, options);
}

} // namespace ToyRenderer

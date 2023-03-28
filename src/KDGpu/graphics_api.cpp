#include "graphics_api.h"

namespace KDGpu {

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

} // namespace KDGpu

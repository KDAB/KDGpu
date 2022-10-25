#include "device.h"

namespace ToyRenderer {

Device::Device(GraphicsApi *api, const DeviceOptions &options)
    : m_api(api)
{
}

Device::~Device()
{
}

} // namespace ToyRenderer

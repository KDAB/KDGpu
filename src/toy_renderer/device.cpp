#include "device.h"

#include <toy_renderer/graphics_api.h>

namespace ToyRenderer {

Device::Device(GraphicsApi *api, const Handle<Adapter_t> &adapterHandle, const DeviceOptions &options)
    : m_api(api)
{
    m_device = m_api->resourceManager()->createDevice(adapterHandle, options);
}

Device::~Device()
{
}

} // namespace ToyRenderer

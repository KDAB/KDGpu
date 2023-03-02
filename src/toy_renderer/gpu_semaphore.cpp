#include "gpu_semaphore.h"

#include <toy_renderer/graphics_api.h>
#include <toy_renderer/resource_manager.h>
#include <toy_renderer/api/api_gpu_semaphore.h>

namespace ToyRenderer {

GpuSemaphore::GpuSemaphore()
{
}

GpuSemaphore::GpuSemaphore(GraphicsApi *api, const Handle<Device_t> &device, const Handle<GpuSemaphore_t> &gpuSemaphore)
    : m_api(api)
    , m_device(device)
    , m_gpuSemaphore(gpuSemaphore)
{
}

GpuSemaphore::~GpuSemaphore()
{
    if (isValid())
        m_api->resourceManager()->deleteGpuSemaphore(handle());
}

GpuSemaphore::GpuSemaphore(GpuSemaphore &&other)
{
    m_api = other.m_api;
    m_device = other.m_device;
    m_gpuSemaphore = other.m_gpuSemaphore;

    other.m_api = nullptr;
    other.m_device = {};
    other.m_gpuSemaphore = {};
}

GpuSemaphore &GpuSemaphore::operator=(GpuSemaphore &&other)
{
    if (this != &other) {
        m_api = other.m_api;
        m_device = other.m_device;
        m_gpuSemaphore = other.m_gpuSemaphore;

        other.m_api = nullptr;
        other.m_device = {};
        other.m_gpuSemaphore = {};
    }
    return *this;
}

} // namespace ToyRenderer

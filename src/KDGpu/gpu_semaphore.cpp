#include "gpu_semaphore.h"

#include <KDGpu/graphics_api.h>
#include <KDGpu/resource_manager.h>
#include <KDGpu/api/api_gpu_semaphore.h>

namespace KDGpu {

GpuSemaphore::GpuSemaphore()
{
}

GpuSemaphore::GpuSemaphore(GraphicsApi *api, const Handle<Device_t> &device, const GpuSemaphoreOptions &options)
    : m_api(api)
    , m_device(device)
    , m_gpuSemaphore(m_api->resourceManager()->createGpuSemaphore(m_device, options))
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
        if (isValid())
            m_api->resourceManager()->deleteGpuSemaphore(handle());

        m_api = other.m_api;
        m_device = other.m_device;
        m_gpuSemaphore = other.m_gpuSemaphore;

        other.m_api = nullptr;
        other.m_device = {};
        other.m_gpuSemaphore = {};
    }
    return *this;
}

} // namespace KDGpu

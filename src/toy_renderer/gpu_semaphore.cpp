#include "gpu_semaphore.h"

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
}

} // namespace ToyRenderer

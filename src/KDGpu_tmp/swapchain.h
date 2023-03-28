#pragma once

#include <kdgpu/handle.h>
#include <kdgpu/texture.h>
#include <kdgpu/kdgpu_export.h>

#include <span>
#include <vector>

namespace KDGpu {

class GraphicsApi;

struct GpuSemaphore_t;
struct Swapchain_t;
struct SwapchainOptions;

class KDGPU_EXPORT Swapchain
{
public:
    Swapchain();
    ~Swapchain();

    Swapchain(Swapchain &&);
    Swapchain &operator=(Swapchain &&);

    Swapchain(const Swapchain &) = delete;
    Swapchain &operator=(const Swapchain &) = delete;

    const Handle<Swapchain_t> &handle() const noexcept { return m_swapchain; }
    bool isValid() const noexcept { return m_swapchain.isValid(); }

    operator Handle<Swapchain_t>() const noexcept { return m_swapchain; }

    const std::vector<Texture> &textures() const { return m_textures; }

    AcquireImageResult getNextImageIndex(uint32_t &imageIndex, const Handle<GpuSemaphore_t> &semaphore = Handle<GpuSemaphore_t>());

private:
    explicit Swapchain(GraphicsApi *api, const Handle<Device_t> &device, const SwapchainOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<Swapchain_t> m_swapchain;
    std::vector<Texture> m_textures;

    friend class Device;
};

} // namespace KDGpu

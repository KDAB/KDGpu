#pragma once

#include <toy_renderer/gpu_core.h>
#include <toy_renderer/handle.h>
#include <toy_renderer/queue_description.h>
#include <toy_renderer/toy_renderer_export.h>

#include <vector>

namespace ToyRenderer {

class GraphicsApi;
class Surface;

struct Adapter_t;
struct CommandBuffer_t;
struct GpuSemaphore_t;
struct Swapchain_t;
struct Fence_t;

struct SubmitOptions {
    std::vector<Handle<CommandBuffer_t>> commandBuffers;
    std::vector<Handle<GpuSemaphore_t>> waitSemaphores;
    std::vector<Handle<GpuSemaphore_t>> signalSemaphores;
    Handle<Fence_t> signalFence;
};

struct SwapchainPresentInfo {
    Handle<Swapchain_t> swapchain;
    uint32_t imageIndex;
};

struct PresentOptions {
    std::vector<Handle<GpuSemaphore_t>> waitSemaphores;
    std::vector<SwapchainPresentInfo> swapchainInfos;
};

class TOY_RENDERER_EXPORT Queue
{
public:
    Queue();
    ~Queue();

    const Handle<Queue_t> &handle() const noexcept { return m_queue; }
    bool isValid() const noexcept { return m_queue.isValid(); }

    operator Handle<Queue_t>() const noexcept { return m_queue; }

    QueueFlags flags() const noexcept { return m_flags; }
    uint32_t timestampValidBits() const noexcept { return m_timestampValidBits; }
    Extent3D minImageTransferGranularity() const noexcept { return m_minImageTransferGranularity; }
    uint32_t queueTypeIndex() const noexcept { return m_queueTypeIndex; }

    void waitUntilIdle();
    void submit(const SubmitOptions &options);

    std::vector<PresentResult> present(const PresentOptions &options);

private:
    Queue(GraphicsApi *api, const QueueDescription &queueDescription);

    GraphicsApi *m_api{ nullptr };

    Handle<Queue_t> m_queue;
    QueueFlags m_flags;
    uint32_t m_timestampValidBits;
    Extent3D m_minImageTransferGranularity;
    uint32_t m_queueTypeIndex;

    friend class Device;
};

} // namespace ToyRenderer

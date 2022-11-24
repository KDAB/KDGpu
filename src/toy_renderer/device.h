#pragma once

#include <toy_renderer/buffer.h>
#include <toy_renderer/device_options.h>
#include <toy_renderer/handle.h>
#include <toy_renderer/queue.h>
#include <toy_renderer/shader_module.h>
#include <toy_renderer/swapchain.h>

#include <toy_renderer/toy_renderer_export.h>

#include <span>
#include <string>
#include <vector>

namespace ToyRenderer {

class Adapter;
class GraphicsApi;
struct Adapter_t;
struct Device_t;

struct BufferOptions;
struct SwapchainOptions;
struct TextureOptions;

class TOY_RENDERER_EXPORT Device
{
public:
    ~Device();

    std::span<Queue> queues() { return m_queues; }

    Swapchain createSwapchain(const SwapchainOptions &options);
    Texture createTexture(const TextureOptions &options);

    // TODO: If initialData is set, upload this to the newly created buffer
    Buffer createBuffer(const BufferOptions &options, void *initialData = nullptr);

    ShaderModule createShaderModule(const std::vector<uint32_t> &code);

private:
    Device(Adapter *adapter, GraphicsApi *api, const DeviceOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    std::vector<Queue> m_queues;

    friend class Adapter;
};

} // namespace ToyRenderer

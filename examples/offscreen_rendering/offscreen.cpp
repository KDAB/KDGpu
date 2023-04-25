#include "offscreen.h"

#include <KDGpu/buffer_options.h>
#include <KDGpu/graphics_pipeline_options.h>
#include <KDGpu/texture_options.h>

#include <spdlog/spdlog.h>

#include <fstream>
#include <string>

namespace KDGpu {

inline std::string assetPath()
{
#if defined(KDGPU_ASSET_PATH)
    return KDGPU_ASSET_PATH;
#else
    return "";
#endif
}

} // namespace KDGpu

Offscreen::Offscreen()
    : m_api(std::make_unique<VulkanGraphicsApi>())
{
    m_instance = m_api->createInstance(InstanceOptions{
            .applicationName = "offscreen_rendering",
            .applicationVersion = SERENITY_MAKE_API_VERSION(0, 1, 0, 0) });

    auto adapter = m_instance.selectAdapter(AdapterDeviceType::Default);
    const auto adapterProperties = adapter->properties();
    SPDLOG_INFO("Using adapter: {}", adapterProperties.deviceName);

    // Create a device and grab the first queue
    m_device = adapter->createDevice();
    m_queue = m_device.queues()[0];

    createRenderTargets();
}

Offscreen::~Offscreen()
{
    cleanupScene();
}

void Offscreen::initializeScene()
{
    // Create a buffer to hold triangle vertex data
    BufferOptions bufferOptions = {
        .size = 3 * 2 * 4 * sizeof(float), // 3 vertices * 2 attributes * 4 float components
        .usage = BufferUsageFlagBits::VertexBufferBit,
        .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
    };
    m_buffer = m_device.createBuffer(bufferOptions);

    // clang-format off
    std::vector<float> vertexData = {
         1.0f, -1.0f, 0.0f, 1.0f, // position
         1.0f,  0.0f, 0.0f, 1.0f, // color
        -1.0f, -1.0f, 0.0f, 1.0f, // position
         0.0f,  1.0f, 0.0f, 1.0f, // color
         0.0f,  1.0f, 0.0f, 1.0f, // position
         0.0f,  0.0f, 1.0f, 1.0f, // color
    };
    // clang-format on
    auto bufferData = m_buffer.map();
    std::memcpy(bufferData, vertexData.data(), vertexData.size() * sizeof(float));
    m_buffer.unmap();

    // Create a vertex shader and fragment shader (spir-v only for now)
    const auto vertexShaderPath = KDGpu::assetPath() + "/shaders/examples/02_hello_triangle/hello_triangle.vert.spv";
    auto vertexShader = m_device.createShaderModule(KDGpu::readShaderFile(vertexShaderPath));

    const auto fragmentShaderPath = KDGpu::assetPath() + "/shaders/examples/02_hello_triangle/hello_triangle.frag.spv";
    auto fragmentShader = m_device.createShaderModule(KDGpu::readShaderFile(fragmentShaderPath));

    // Create a pipeline layout (array of bind group layouts)
    m_pipelineLayout = m_device.createPipelineLayout();

    // Create a pipeline
    // clang-format off
    GraphicsPipelineOptions pipelineOptions = {
        .shaderStages = {
            { .shaderModule = vertexShader, .stage = ShaderStageFlagBits::VertexBit },
            { .shaderModule = fragmentShader, .stage = ShaderStageFlagBits::FragmentBit }
        },
        .layout = m_pipelineLayout,
        .vertex = {
            .buffers = {
                { .binding = 0, .stride = 2 * 4 * sizeof(float) }
            },
            .attributes = {
                { .location = 0, .binding = 0, .format = Format::R32G32B32A32_SFLOAT }, // Position
                { .location = 1, .binding = 0, .format = Format::R32G32B32A32_SFLOAT, .offset = 4 * sizeof(float) } // Color
            }
        },
        .renderTargets = {
            { .format = m_colorFormat }
        },
        .depthStencil = {
            .format = m_depthFormat,
            .depthWritesEnabled = true,
            .depthCompareOperation = CompareOperation::Less
        }
    };
    // clang-format on
    m_pipeline = m_device.createGraphicsPipeline(pipelineOptions);

    // Most of the render pass is the same between frames. The only thing that changes, is which image
    // of the swapchain we wish to render to. So set up what we can here, and in the render loop we will
    // just update the color texture view.
    // clang-format off
    m_opaquePassOptions = {
        .colorAttachments = {
            {
                .view = m_colorTextureView,
                .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f }
            }
        },
        .depthStencilAttachment = {
            .view = m_depthTextureView,
        }
    };
    // clang-format on
}

void Offscreen::cleanupScene()
{
    m_pipeline = {};
    m_pipelineLayout = {};
    m_buffer = {};
    m_commandBuffer = {};
}

void Offscreen::resize(uint32_t width, uint32_t height)
{
    if (width == m_width && height == m_height)
        return;

    m_width = width;
    m_height = height;
    createRenderTargets();
}

void Offscreen::updateScene()
{
}

void Offscreen::render()
{
    auto commandRecorder = m_device.createCommandRecorder();
    auto opaquePass = commandRecorder.beginRenderPass(m_opaquePassOptions);
    opaquePass.setPipeline(m_pipeline);
    opaquePass.setVertexBuffer(0, m_buffer);
    const DrawCommand drawCmd = { .vertexCount = 3 };
    opaquePass.draw(drawCmd);
    opaquePass.end();

    // Copy from the color render target to the CPU visible color texture

    // Insert a texture memory barrier to ensure the rendering to the color render target
    // is completed and to transition it into a layout suitable for copying from
    const TextureMemoryBarrierOptions copySourcePrepareBarrierOptions = {
        .srcStages = PipelineStageFlagBit::TransferBit,
        .srcMask = AccessFlagBit::MemoryReadBit,
        .dstStages = PipelineStageFlagBit::TransferBit,
        .dstMask = AccessFlagBit::TransferReadBit,
        .oldLayout = TextureLayout::ColorAttachmentOptimal,
        .newLayout = TextureLayout::TransferSrcOptimal,
        .texture = m_colorTexture,
        .range = { .aspectMask = TextureAspectFlagBits::ColorBit }
    };
    commandRecorder.textureMemoryBarrier(copySourcePrepareBarrierOptions);

    // Insert another texture memory barrier to transition the destination cpu visible
    // texture into a suitable layout for copying into
    const TextureMemoryBarrierOptions copyDestinationPrepareBarrierOptions = {
        .srcStages = PipelineStageFlagBit::TransferBit,
        .srcMask = AccessFlagBit::None,
        .dstStages = PipelineStageFlagBit::TransferBit,
        .dstMask = AccessFlagBit::TransferWriteBit,
        .oldLayout = TextureLayout::Undefined,
        .newLayout = TextureLayout::TransferDstOptimal,
        .texture = m_cpuColorTexture,
        .range = { .aspectMask = TextureAspectFlagBits::ColorBit }
    };
    commandRecorder.textureMemoryBarrier(copyDestinationPrepareBarrierOptions);

    // Perform the copy operation
    // clang-format off
    const TextureToTextureCopy copyOptions = {
        .srcTexture = m_colorTexture,
        .srcLayout = TextureLayout::TransferSrcOptimal,
        .dstTexture = m_cpuColorTexture,
        .dstLayout = TextureLayout::TransferDstOptimal,
        .regions = {{
            .extent = { .width = m_width, .height = m_height, .depth = 1 }
        }}
    };
    // clang-format on
    commandRecorder.copyTextureToTexture(copyOptions);

    // Transition the destination texture to general layout so that we can map it to the cpu
    // address space later.
    const TextureMemoryBarrierOptions copyDestinationCleanupBarrierOptions = {
        .srcStages = PipelineStageFlagBit::TransferBit,
        .srcMask = AccessFlagBit::TransferWriteBit,
        .dstStages = PipelineStageFlagBit::TransferBit,
        .dstMask = AccessFlagBit::MemoryReadBit,
        .oldLayout = TextureLayout::TransferDstOptimal,
        .newLayout = TextureLayout::General,
        .texture = m_cpuColorTexture,
        .range = { .aspectMask = TextureAspectFlagBits::ColorBit }
    };
    commandRecorder.textureMemoryBarrier(copyDestinationCleanupBarrierOptions);

    // Transition the color target back to the color attachment optimal layout, ready
    // to render again later.
    const TextureMemoryBarrierOptions copySourceCleanupBarrierOptions = {
        .srcStages = PipelineStageFlagBit::TransferBit,
        .srcMask = AccessFlagBit::TransferReadBit,
        .dstStages = PipelineStageFlagBit::TransferBit,
        .dstMask = AccessFlagBit::MemoryReadBit,
        .oldLayout = TextureLayout::TransferSrcOptimal,
        .newLayout = TextureLayout::ColorAttachmentOptimal,
        .texture = m_colorTexture,
        .range = { .aspectMask = TextureAspectFlagBits::ColorBit }
    };
    commandRecorder.textureMemoryBarrier(copySourceCleanupBarrierOptions);

    m_commandBuffer = commandRecorder.finish();

    SubmitOptions submitOptions = {
        .commandBuffers = { m_commandBuffer }
    };
    m_queue.submit(submitOptions);
    m_queue.waitUntilIdle();

    // Map the host texture to cpu address space so we can save it to disk
    const auto subresourceLayout = m_cpuColorTexture.getSubresourceLayout();
    const uint8_t *data = static_cast<uint8_t *>(m_cpuColorTexture.map());
    data += subresourceLayout.offset;

    const std::string filename("test.ppm");
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    file << "P6\n"
         << m_width << "\n"
         << m_height << "\n"
         << 255 << "\n"; // PPM file header

    for (uint32_t y = 0; y < m_height; ++y) {
        uint32_t *texel = (uint32_t *)data;
        for (uint32_t x = 0; x < m_width; ++x) {
            file.write((char *)texel, 3); // Output RGB for current texel
            ++texel;
        }
        data += subresourceLayout.rowPitch;
    }
    file.close();

    SPDLOG_INFO("Saved image to disk as {}", filename);
    m_cpuColorTexture.unmap();
}

void Offscreen::createRenderTargets()
{
    // Create a color texture to use as our render target
    TextureOptions colorTextureOptions = {
        .type = TextureType::TextureType2D,
        .format = m_colorFormat,
        .extent = { m_width, m_height, 1 },
        .mipLevels = 1,
        .samples = SampleCountFlagBits::Samples1Bit, // TODO: Expose samples setting?
        .usage = TextureUsageFlagBits::ColorAttachmentBit | TextureUsageFlagBits::TransferSrcBit,
        .memoryUsage = MemoryUsage::GpuOnly
    };
    m_colorTexture = m_device.createTexture(colorTextureOptions);
    m_colorTextureView = m_colorTexture.createView();

    // Create a depth texture to use for depth-correct rendering
    TextureOptions depthTextureOptions = {
        .type = TextureType::TextureType2D,
        .format = m_depthFormat,
        .extent = { m_width, m_height, 1 },
        .mipLevels = 1,
        .samples = SampleCountFlagBits::Samples1Bit, // TODO: Expose samples setting?
        .usage = TextureUsageFlagBits::DepthStencilAttachmentBit,
        .memoryUsage = MemoryUsage::GpuOnly
    };
    m_depthTexture = m_device.createTexture(depthTextureOptions);
    m_depthTextureView = m_depthTexture.createView();

    // Create a color texture that is host visible and in linear layout. We will copy into this.
    TextureOptions cpuColorTextureOptions = {
        .type = TextureType::TextureType2D,
        .format = m_colorFormat,
        .extent = { m_width, m_height, 1 },
        .mipLevels = 1,
        .samples = SampleCountFlagBits::Samples1Bit,
        .tiling = TextureTiling::Linear, // Linear so we can manipulate it on the host
        .usage = TextureUsageFlagBits::TransferDstBit,
        .memoryUsage = MemoryUsage::CpuOnly
    };
    m_cpuColorTexture = m_device.createTexture(cpuColorTextureOptions);
}

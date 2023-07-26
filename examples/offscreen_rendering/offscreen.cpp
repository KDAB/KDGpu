/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "offscreen.h"

#include <KDGpuExample/kdgpuexample.h>

#include <KDGpu/bind_group_options.h>
#include <KDGpu/bind_group_layout_options.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/graphics_pipeline_options.h>
#include <KDGpu/texture_options.h>

#include <KDUtils/elapsedtimer.h>

#include <glm/gtc/matrix_transform.hpp>

#include <spdlog/spdlog.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <fstream>
#include <string>

struct ImageData {
    uint32_t width{ 0 };
    uint32_t height{ 0 };
    const uint8_t *pixelData{ nullptr };
    DeviceSize byteSize{ 0 };
    Format format{ Format::R8G8B8A8_UNORM };
};

namespace KDGpu {

inline std::string assetPath()
{
#if defined(KDGPU_ASSET_PATH)
    return KDGPU_ASSET_PATH;
#else
    return "";
#endif
}

ImageData loadImage(const std::string &path)
{
    int texChannels;
    int _width = 0, _height = 0;
    std::string texturePath = path;
#ifdef PLATFORM_WIN32
    // STB fails to load if path is /C:/... instead of C:/...
    if (texturePath.rfind("/", 0) == 0)
        texturePath = texturePath.substr(1);
#endif
    auto _data = stbi_load(texturePath.c_str(), &_width, &_height, &texChannels, STBI_rgb_alpha);
    if (_data == nullptr) {
        SPDLOG_WARN("Failed to load texture {} {}", path, stbi_failure_reason());
        return {};
    }
    SPDLOG_DEBUG("Texture dimensions: {} x {}", _width, _height);

    return ImageData{
        .width = static_cast<uint32_t>(_width),
        .height = static_cast<uint32_t>(_height),
        .pixelData = static_cast<uint8_t *>(_data),
        .byteSize = 4 * static_cast<DeviceSize>(_width) * static_cast<DeviceSize>(_height)
    };
}

void writeImage(const std::string &path, const ImageData &image)
{
    const uint32_t channelCount = 4;
    stbi_write_png(path.c_str(), image.width, image.height, channelCount, image.pixelData, image.width * channelCount);
}

} // namespace KDGpu

//![7]
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
    m_device = adapter->createDevice(DeviceOptions{ .requestedFeatures = adapter->features() });
    m_queue = m_device.queues()[0];

    createRenderTargets();
}
//![7]

Offscreen::~Offscreen()
{
    cleanupScene();
}

void Offscreen::initializeScene()
{
    // Create a texture to hold the image data
    {
        // Load the image data and size
        ImageData image = loadImage(KDGpu::assetPath() + "/textures/point-simple-large.png");

        const TextureOptions textureOptions = {
            .type = TextureType::TextureType2D,
            .format = image.format,
            .extent = { .width = image.width, .height = image.height, .depth = 1 },
            .mipLevels = 1,
            .usage = TextureUsageFlagBits::SampledBit | TextureUsageFlagBits::TransferDstBit,
            .memoryUsage = MemoryUsage::GpuOnly,
            .initialLayout = TextureLayout::Undefined
        };
        m_pointTexture = m_device.createTexture(textureOptions);

        // Upload the texture data and transition to ShaderReadOnlyOptimal
        // clang-format off
        const std::vector<BufferTextureCopyRegion> regions = {{
            .textureSubResource = { .aspectMask = TextureAspectFlagBits::ColorBit },
            .textureExtent = { .width = image.width, .height = image.height, .depth = 1 }
        }};
        // clang-format on
        const TextureUploadOptions uploadOptions = {
            .destinationTexture = m_pointTexture,
            .dstStages = PipelineStageFlagBit::AllGraphicsBit,
            .dstMask = AccessFlagBit::MemoryReadBit,
            .data = image.pixelData,
            .byteSize = image.byteSize,
            .oldLayout = TextureLayout::Undefined,
            .newLayout = TextureLayout::ShaderReadOnlyOptimal,
            .regions = regions
        };
        //![7]
        m_stagingBuffers.emplace_back(m_queue.uploadTextureData(uploadOptions));
        //![7]

        // Create a view and sampler
        m_pointTextureView = m_pointTexture.createView();
        //![1]
        m_pointSampler = m_device.createSampler(SamplerOptions{ .magFilter = FilterMode::Linear, .minFilter = FilterMode::Linear });
        //![1]
    }

    // Create a vertex shader and fragment shader (spir-v only for now)
    const auto vertexShaderPath = KDGpu::assetPath() + "/shaders/examples/offscreen_rendering/plot.vert.spv";
    auto vertexShader = m_device.createShaderModule(KDGpuExample::readShaderFile(vertexShaderPath));

    const auto fragmentShaderPath = KDGpu::assetPath() + "/shaders/examples/offscreen_rendering/plot.frag.spv";
    auto fragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(fragmentShaderPath));

    // Create bind group layout consisting of a single binding holding a combined texture-sampler
    // clang-format off
    const BindGroupLayoutOptions pointTextureBindGroupLayoutOptions = {
        .bindings = {{
            .binding = 0,
            .resourceType = ResourceBindingType::CombinedImageSampler,
            .shaderStages = ShaderStageFlags(ShaderStageFlagBits::FragmentBit)
        }}
    };
    // clang-format on
    const BindGroupLayout pointTextureBindGroupLayout = m_device.createBindGroupLayout(pointTextureBindGroupLayoutOptions);

    // Create a bindGroup to hold the uniform containing the texture and sampler
    // clang-format off
    const BindGroupOptions pointTextureBindGroupOptions = {
        .layout = pointTextureBindGroupLayout,
        .resources = {{
            .binding = 0,
            .resource = TextureViewSamplerBinding{ .textureView = m_pointTextureView, .sampler = m_pointSampler }
        }}
    };
    // clang-format on
    m_pointTextureBindGroup = m_device.createBindGroup(pointTextureBindGroupOptions);

    // Create a buffer to hold the transformation matrix
    {
        BufferOptions bufferOptions = {
            .size = sizeof(glm::mat4),
            .usage = BufferUsageFlagBits::UniformBufferBit,
            .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
        };
        m_projBuffer = m_device.createBuffer(bufferOptions);

        // Set the default to be NDC (but y up)
        setProjection(-1.0f, 1.0f, -1.0f, 1.0f);
    }

    // Create bind group layout consisting of a single binding holding a UBO for the transform
    // clang-format off
    const BindGroupLayoutOptions transformBindGroupLayoutOptions = {
        .bindings = {{
            .binding = 0,
            .resourceType = ResourceBindingType::UniformBuffer,
            .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit)
        }}
    };
    // clang-format on
    const BindGroupLayout transformBindGroupLayout = m_device.createBindGroupLayout(transformBindGroupLayoutOptions);

    const BindGroupOptions transformBindGroupOptions = {
        .layout = transformBindGroupLayout,
        .resources = { { .binding = 0,
                         .resource = UniformBufferBinding{ .buffer = m_projBuffer } } }
    };
    // clang-format on
    m_transformBindGroup = m_device.createBindGroup(transformBindGroupOptions);

    // Create a pipeline layout (array of bind group layouts)
    const PipelineLayoutOptions pipelineLayoutOptions = {
        .bindGroupLayouts = { pointTextureBindGroupLayout, transformBindGroupLayout }
    };
    m_pipelineLayout = m_device.createPipelineLayout(pipelineLayoutOptions);

    // Create a pipeline
    // clang-format off
    //![5]
    const GraphicsPipelineOptions pipelineOptions = {
        .shaderStages = {
            { .shaderModule = vertexShader, .stage = ShaderStageFlagBits::VertexBit },
            { .shaderModule = fragmentShader, .stage = ShaderStageFlagBits::FragmentBit }
        },
        .layout = m_pipelineLayout,
        .vertex = {
            .buffers = {
                { .binding = 0, .stride = sizeof(Offscreen::Vertex) }
            },
            .attributes = {
                { .location = 0, .binding = 0, .format = Format::R32G32_SFLOAT }, // Position
                { .location = 1, .binding = 0, .format = Format::R32G32B32A32_SFLOAT, .offset = sizeof(glm::vec2) } // Color
            }
        },
        .renderTargets = {{
            .format = m_colorFormat,
            .blending = {
                .blendingEnabled = true,
                .color = {
                    .srcFactor = BlendFactor::SrcAlpha,
                    .dstFactor = BlendFactor::OneMinusSrcAlpha
                },
                .alpha = {
                    .srcFactor = BlendFactor::SrcAlpha,
                    .dstFactor = BlendFactor::OneMinusSrcAlpha
                }
            }
        }},
        .depthStencil = {
            .format = m_depthFormat,
            .depthTestEnabled = false,
            .depthWritesEnabled = false,
            .depthCompareOperation = CompareOperation::Always
        },
        .primitive = {
            .topology = PrimitiveTopology::PointList
        },
        .multisample = {
            .samples = m_samples
        }
    };
    //![5]
    // clang-format on
    m_pipeline = m_device.createGraphicsPipeline(pipelineOptions);
}

void Offscreen::cleanupScene()
{
    m_transformBindGroup = {};
    m_projBuffer = {};
    m_pointTextureBindGroup = {};
    m_pointSampler = {};
    m_pointTextureView = {};
    m_pointTexture = {};
    m_msaaColorTextureView = {};
    m_msaaColorTexture = {};
    m_colorTextureView = {};
    m_colorTexture = {};
    m_depthTextureView = {};
    m_depthTexture = {};
    m_pipeline = {};
    m_pipelineLayout = {};
    m_dataBuffer = {};
    m_commandBuffer = {};
    m_stagingBuffers.clear();
}

void Offscreen::resize(uint32_t width, uint32_t height)
{
    if (width == m_width && height == m_height)
        return;

    m_width = width;
    m_height = height;
    createRenderTargets();
}

//![6]
void Offscreen::setData(const std::vector<Offscreen::Vertex> &data)
{
    m_pointCount = data.size();

    const DeviceSize dataByteSize = data.size() * sizeof(Offscreen::Vertex);
    BufferOptions bufferOptions = {
        .size = dataByteSize,
        .usage = BufferUsageFlagBits::VertexBufferBit | BufferUsageFlagBits::TransferDstBit,
        .memoryUsage = MemoryUsage::GpuOnly
    };
    m_dataBuffer = m_device.createBuffer(bufferOptions);
    const BufferUploadOptions uploadOptions = {
        .destinationBuffer = m_dataBuffer,
        .dstStages = PipelineStageFlagBit::VertexAttributeInputBit,
        .dstMask = AccessFlagBit::VertexAttributeReadBit,
        .data = data.data(),
        .byteSize = dataByteSize
    };

    // Initiate the data upload. We note the upload details so that we can
    // test to see when it is safe to destroy the staging buffer. We will check
    // at the end of each render function.
    m_stagingBuffers.emplace_back(m_queue.uploadBufferData(uploadOptions));
}
//![6]

//![4]
void Offscreen::setProjection(float left, float right, float bottom, float top)
{
    // NB: We flip bottom and top since Vulkan (and KDGpu) invert the y vs OpenGL
    m_proj = glm::ortho(left, right, top, bottom);
    auto bufferData = m_projBuffer.map();
    std::memcpy(bufferData, &m_proj, sizeof(glm::mat4));
    m_projBuffer.unmap();
}
//![4]

void Offscreen::releaseStagingBuffers()
{
    // Loop over any staging buffers and see if the corresponding fence has been signalled.
    // If so, we can dispose of them
    const auto removedCount = std::erase_if(m_stagingBuffers, [](const UploadStagingBuffer &stagingBuffer) {
        return stagingBuffer.fence.status() == FenceStatus::Signalled;
    });
    if (removedCount)
        SPDLOG_INFO("Released {} staging buffers", removedCount);
}

void Offscreen::render(const std::string &baseFilename)
{
    KDUtils::ElapsedTimer elapsed;
    elapsed.start();

    // Render the scene to the offscreen color texture target
    auto commandRecorder = m_device.createCommandRecorder();
    auto renderPass = commandRecorder.beginRenderPass(m_renderPassOptions);
    renderPass.setPipeline(m_pipeline);
    renderPass.setBindGroup(0, m_pointTextureBindGroup);
    renderPass.setBindGroup(1, m_transformBindGroup);
    renderPass.setVertexBuffer(0, m_dataBuffer);
    const DrawCommand drawCmd = { .vertexCount = m_pointCount };
    renderPass.draw(drawCmd);
    renderPass.end();

    // Copy from the color render target to the CPU visible color texture. The barriers ensure
    // that we correctly serialize the operations performed on the GPU and also act to transition
    // the textures into the correct layout for each step. See the explanations in the
    // createRenderTargets() function for more information.
    //![11]
    commandRecorder.textureMemoryBarrier(m_barriers[uint8_t(TextureBarriers::CopySrcPre)]);
    commandRecorder.textureMemoryBarrier(m_barriers[uint8_t(TextureBarriers::CopyDstPre)]);
    commandRecorder.copyTextureToTexture(m_copyOptions);
    commandRecorder.textureMemoryBarrier(m_barriers[uint8_t(TextureBarriers::CopyDstPost)]);
    commandRecorder.textureMemoryBarrier(m_barriers[uint8_t(TextureBarriers::CopySrcPost)]);
    //![11]

    // Finish recording and submit
    m_commandBuffer = commandRecorder.finish();
    const SubmitOptions submitOptions = {
        .commandBuffers = { m_commandBuffer }
    };
    m_queue.submit(submitOptions);
    m_queue.waitUntilIdle();

    SPDLOG_INFO("Render and copy completed in {} s", elapsed.nsecElapsed() / 1.0e9);

    // Map the host texture to cpu address space so we can save it to disk
    const auto subresourceLayout = m_cpuColorTexture.getSubresourceLayout();
    const uint8_t *data = static_cast<uint8_t *>(m_cpuColorTexture.map());
    data += subresourceLayout.offset;

    SPDLOG_INFO("Mapping completed in {} s", elapsed.nsecElapsed() / 1.0e9);

// #define KDGPU_OFFSCREEN_SAVE_AS_PPM
#if defined(KDGPU_OFFSCREEN_SAVE_AS_PPM)
    // For this example we just output the RGB channels to disk as a PPM file.
    const std::string filename = baseFilename + ".ppm";
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
#else
    // Save as PNG
    const std::string filename = baseFilename + ".png";
    const ImageData imageData = {
        .width = m_width,
        .height = m_height,
        .pixelData = data,
        .byteSize = subresourceLayout.size,
    };
    writeImage(filename, imageData);
#endif

    SPDLOG_INFO("Saving completed in {} s", elapsed.nsecElapsed() / 1.0e9);
    SPDLOG_INFO("Saved image to disk as {}", filename);

    m_cpuColorTexture.unmap();

    // See if we can release any staging buffers used for uploads. As we are waiting
    // for the queue to become idle above, we should always be able to release here.
    releaseStagingBuffers();
}

void Offscreen::createRenderTargets()
{
    //![8]
    // Create a color texture to use as our color render target
    const TextureOptions msaaColorTextureOptions = {
        .type = TextureType::TextureType2D,
        .format = m_colorFormat,
        .extent = { m_width, m_height, 1 },
        .mipLevels = 1,
        .samples = m_samples,
        .usage = TextureUsageFlagBits::ColorAttachmentBit,
        .memoryUsage = MemoryUsage::GpuOnly
    };
    m_msaaColorTexture = m_device.createTexture(msaaColorTextureOptions);
    m_msaaColorTextureView = m_msaaColorTexture.createView();

    // Create a color texture to use as our resolve render target
    const TextureOptions colorTextureOptions = {
        .type = TextureType::TextureType2D,
        .format = m_colorFormat,
        .extent = { m_width, m_height, 1 },
        .mipLevels = 1,
        .samples = SampleCountFlagBits::Samples1Bit,
        .usage = TextureUsageFlagBits::ColorAttachmentBit | TextureUsageFlagBits::TransferSrcBit,
        .memoryUsage = MemoryUsage::GpuOnly
    };
    m_colorTexture = m_device.createTexture(colorTextureOptions);
    m_colorTextureView = m_colorTexture.createView();
    //![8]

    // Create a depth texture to use for depth-correct rendering
    const TextureOptions depthTextureOptions = {
        .type = TextureType::TextureType2D,
        .format = m_depthFormat,
        .extent = { m_width, m_height, 1 },
        .mipLevels = 1,
        .samples = m_samples,
        .usage = TextureUsageFlagBits::DepthStencilAttachmentBit,
        .memoryUsage = MemoryUsage::GpuOnly
    };
    m_depthTexture = m_device.createTexture(depthTextureOptions);
    m_depthTextureView = m_depthTexture.createView();

    // clang-format off
    //![9]
    m_renderPassOptions = {
        .colorAttachments = {
            {
                .view = m_msaaColorTextureView,
                .resolveView = m_colorTextureView,
                .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f }
            }
        },
        .depthStencilAttachment = {
            .view = m_depthTextureView,
        },
        .samples = m_samples
    };
    //![9]
    // clang-format on

    //![10]
    // Create a color texture that is host visible and in linear layout. We will copy into this.
    const TextureOptions cpuColorTextureOptions = {
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
    //![10]

    // Setup the options for the memory barriers that will be used to serialize the
    // memory accesses and transition the textures into the correct layouts for each step.
    // These will be the same for every call to render() unless we have to resize and
    // hence recreate the textures we are rendering to and copying between.

    //![12]
    // Insert a texture memory barrier to ensure the rendering to the color render target
    // is completed and to transition it into a layout suitable for copying from
    m_barriers[uint8_t(TextureBarriers::CopySrcPre)] = {
        .srcStages = PipelineStageFlagBit::TransferBit,
        .srcMask = AccessFlagBit::MemoryReadBit,
        .dstStages = PipelineStageFlagBit::TransferBit,
        .dstMask = AccessFlagBit::TransferReadBit,
        .oldLayout = TextureLayout::ColorAttachmentOptimal,
        .newLayout = TextureLayout::TransferSrcOptimal,
        .texture = m_colorTexture,
        .range = { .aspectMask = TextureAspectFlagBits::ColorBit }
    };

    // Insert another texture memory barrier to transition the destination cpu visible
    // texture into a suitable layout for copying into
    m_barriers[uint8_t(TextureBarriers::CopyDstPre)] = {
        .srcStages = PipelineStageFlagBit::TransferBit,
        .srcMask = AccessFlagBit::None,
        .dstStages = PipelineStageFlagBit::TransferBit,
        .dstMask = AccessFlagBit::TransferWriteBit,
        .oldLayout = TextureLayout::Undefined,
        .newLayout = TextureLayout::TransferDstOptimal,
        .texture = m_cpuColorTexture,
        .range = { .aspectMask = TextureAspectFlagBits::ColorBit }
    };

    // Transition the destination texture to general layout so that we can map it to the cpu
    // address space later.
    m_barriers[uint8_t(TextureBarriers::CopyDstPost)] = {
        .srcStages = PipelineStageFlagBit::TransferBit,
        .srcMask = AccessFlagBit::TransferWriteBit,
        .dstStages = PipelineStageFlagBit::TransferBit,
        .dstMask = AccessFlagBit::MemoryReadBit,
        .oldLayout = TextureLayout::TransferDstOptimal,
        .newLayout = TextureLayout::General,
        .texture = m_cpuColorTexture,
        .range = { .aspectMask = TextureAspectFlagBits::ColorBit }
    };

    // Transition the color target back to the color attachment optimal layout, ready
    // to render again later.
    m_barriers[uint8_t(TextureBarriers::CopySrcPost)] = {
        .srcStages = PipelineStageFlagBit::TransferBit,
        .srcMask = AccessFlagBit::TransferReadBit,
        .dstStages = PipelineStageFlagBit::TransferBit,
        .dstMask = AccessFlagBit::MemoryReadBit,
        .oldLayout = TextureLayout::TransferSrcOptimal,
        .newLayout = TextureLayout::ColorAttachmentOptimal,
        .texture = m_colorTexture,
        .range = { .aspectMask = TextureAspectFlagBits::ColorBit }
    };
    //![12]

    // Likewise, we can specify the copy operation parameters once here and reuse them many
    // times in calls to render().
    // clang-format off
    //![13]
    m_copyOptions = {
        .srcTexture = m_colorTexture,
        .srcLayout = TextureLayout::TransferSrcOptimal,
        .dstTexture = m_cpuColorTexture,
        .dstLayout = TextureLayout::TransferDstOptimal,
        .regions = {{
            .extent = { .width = m_width, .height = m_height, .depth = 1 }
        }}
    };
    //![13]
    // clang-format on
}

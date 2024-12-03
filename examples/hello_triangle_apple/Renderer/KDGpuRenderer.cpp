/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "KDGpuRenderer.h"

#include <KDGpu/texture_options.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/bind_group_layout_options.h>
#include <KDGpu/bind_group_options.h>
#include <KDGpu/graphics_pipeline_options.h>
#include <KDGpu/swapchain_options.h>

#include <iostream>
#include <fstream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace KDGpu;

namespace {

std::vector<uint32_t> readShaderFile(const std::string &filename)
{
    std::ifstream inputFile(filename, std::ios::binary);

    // Check if the file is open
    if (!inputFile.is_open()) {
        SPDLOG_CRITICAL("Failed to open file {}", filename);
        throw std::runtime_error("Failed to open file");
    }

    // Read the contents of the binary file into a vector of bytes
    std::vector<char> fileContent((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());

    // Close the file
    inputFile.close();

    std::vector<uint32_t> buffer(fileContent.size() / 4);
    std::memcpy(buffer.data(), fileContent.data(), fileContent.size());

    return buffer;
}

std::shared_ptr<spdlog::logger> createLogger(const std::string &name)
{
    auto logger = spdlog::get(name);
    if (logger)
        return logger;
    logger = spdlog::stdout_color_mt(name);
    return logger;
}

} // namespace

KDGpuRenderer::KDGpuRenderer(const KDGpu::SurfaceOptions &options, const std::string &pathToShaderFolder)
    : _api(std::make_unique<VulkanGraphicsApi>())
    , _pathToShaderFolder(pathToShaderFolder)
{
    KDGpu::Logger::setLoggerFactory(createLogger);
    _logger = createLogger("app");
    _logger->set_level(spdlog::level::info);

    _instance = _api->createInstance(InstanceOptions{
            .applicationName = "hello_triangle_apple",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0),
    });
    _surface = _instance.createSurface(options);
    _adapter = _instance.selectAdapter(AdapterDeviceType::Default);
    if (!_adapter)
        throw std::runtime_error("Unable to find a default GPU. Aborting...");
    _device = _adapter->createDevice();
    _queue = _device.queues()[0];

    createSwapchain();
    createBuffers();
    createPipeline();
}

void KDGpuRenderer::createSwapchain()
{
    const AdapterSwapchainProperties swapchainProperties = _device.adapter()->swapchainProperties(_surface);

    // Create a swapchain of images that we will render to.
    const SwapchainOptions swapchainOptions = {
        .surface = _surface.handle(),
        .minImageCount = getSuitableImageCount(swapchainProperties.capabilities),
        .imageExtent = { .width = _width, .height = _height },
        .oldSwapchain = _swapchain,
    };

    _swapchain = _device.createSwapchain(swapchainOptions);
    SPDLOG_LOGGER_INFO(_logger, "Created swapchain with {} images", _swapchain.textures().size());

    const auto &swapchainTextures = _swapchain.textures();
    const auto swapchainTextureCount = swapchainTextures.size();

    _swapchainViews.clear();
    _swapchainViews.reserve(swapchainTextureCount);
    for (uint32_t i = 0; i < swapchainTextureCount; ++i) {
        auto view = swapchainTextures[i].createView({ .format = swapchainOptions.format });
        _swapchainViews.push_back(std::move(view));
    }

    // Create a depth texture to use for rendering
    const TextureOptions depthTextureOptions = {
        .type = TextureType::TextureType2D,
        .format = Format::D32_SFLOAT_S8_UINT,
        .extent = { _width, _height, 1 },
        .mipLevels = 1,
        .usage = TextureUsageFlagBits::DepthStencilAttachmentBit,
        .memoryUsage = MemoryUsage::GpuOnly
    };
    _depthTexture = _device.createTexture(depthTextureOptions);
    _depthTextureView = _depthTexture.createView();
    SPDLOG_LOGGER_INFO(_logger, "Created depth texture");

    _swapchainFormat = swapchainOptions.format;
    _depthTextureFormat = depthTextureOptions.format;
}

void KDGpuRenderer::createBuffers()
{
    _vertexBuffer = _device.createBuffer(BufferOptions{
            .size = 3 * 2 * 4 * sizeof(float), // 3 vertices * 2 attributes * 4 float components
            .usage = BufferUsageFlagBits::VertexBufferBit,
            .memoryUsage = MemoryUsage::CpuToGpu, // So we can map it to CPU address space
    });

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
    {
        void *bufferData = _vertexBuffer.map();
        std::memcpy(bufferData, vertexData.data(), vertexData.size() * sizeof(float));
        _vertexBuffer.unmap();
    }
    SPDLOG_LOGGER_INFO(_logger, "Created vertex buffer");

    _cameraUBOBuffer = _device.createBuffer(BufferOptions{
            .size = 16 * sizeof(float), // 1 * mat4x4
            .usage = BufferUsageFlagBits::UniformBufferBit,
            .memoryUsage = MemoryUsage::CpuToGpu, // So we can map it to CPU address space
    });
    {
        void *bufferData = _cameraUBOBuffer.map();
        glm::mat4 m(1.0);
        std::memcpy(bufferData, &m, 16 * sizeof(float));
        _cameraUBOBuffer.unmap();
    }
    SPDLOG_LOGGER_INFO(_logger, "Created camera UBO buffer");
}

void KDGpuRenderer::createPipeline()
{
    const auto vertexShaderPath = _pathToShaderFolder + "/hello_triangle.vert.spv";
    _vertexShader = _device.createShaderModule(readShaderFile(vertexShaderPath));

    const auto fragmentShaderPath = _pathToShaderFolder + "/hello_triangle.frag.spv";
    _fragmentShader = _device.createShaderModule(readShaderFile(fragmentShaderPath));

    _bindGroupLayout = _device.createBindGroupLayout(BindGroupLayoutOptions{
            .bindings = {
                    { // Camera uniforms
                      .binding = 0,
                      .count = 1,
                      .resourceType = ResourceBindingType::UniformBuffer,
                      .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit) },
            },
    });

    // Create a pipeline layout (array of bind group layouts)
    _pipelineLayout = _device.createPipelineLayout(PipelineLayoutOptions{
            .bindGroupLayouts = { _bindGroupLayout },
    });

    // Create a pipeline
    _pipeline = _device.createGraphicsPipeline(GraphicsPipelineOptions{
            .shaderStages = {
                    { .shaderModule = _vertexShader.handle(), .stage = ShaderStageFlagBits::VertexBit },
                    { .shaderModule = _fragmentShader.handle(), .stage = ShaderStageFlagBits::FragmentBit },
            },
            .layout = _pipelineLayout.handle(),
            .vertex = {
                    .buffers = {
                            { .binding = 0, .stride = 2 * 4 * sizeof(float) },
                    },
                    .attributes = {
                            { .location = 0, .binding = 0, .format = Format::R32G32B32A32_SFLOAT }, // Position
                            { .location = 1, .binding = 0, .format = Format::R32G32B32A32_SFLOAT, .offset = 4 * sizeof(float) }, // Color
                    },
            },
            .renderTargets = {
                    { .format = _swapchainFormat },
            },
            .depthStencil = {
                    .format = _depthTextureFormat,
                    .depthWritesEnabled = true,
                    .depthCompareOperation = CompareOperation::Less,
            },
    });

    _bindGroup = _device.createBindGroup(BindGroupOptions{
            .layout = _bindGroupLayout,
            .resources = {
                    {
                            .binding = 0,
                            .resource = UniformBufferBinding{ .buffer = _cameraUBOBuffer },
                    },
            },
    });

    // Update BindGroup for binding 0
    _bindGroup.update(BindGroupEntry{ .binding = 0, .resource = UniformBufferBinding{ .buffer = _cameraUBOBuffer } });

    _imageAvailableSemaphore = _device.createGpuSemaphore();
    _renderCompleteSemaphore = _device.createGpuSemaphore();
    _frameInFlightFence = _device.createFence(FenceOptions{ .createSignalled = true });
}

void KDGpuRenderer::frame()
{
    _frameInFlightFence.reset();

    uint32_t currentImageIndex = 0;
    AcquireImageResult result = _swapchain.getNextImageIndex(currentImageIndex, _imageAvailableSemaphore);
    if (result == AcquireImageResult::OutOfDate) {
        // This can happen when swapchain was resized
        // We need to recreate the swapchain and retry
        createSwapchain();
        result = _swapchain.getNextImageIndex(currentImageIndex, _imageAvailableSemaphore);
    }
    if (result != AcquireImageResult::Success) {
        SPDLOG_LOGGER_ERROR(_logger, "Unable to acquire swapchain image");
        return;
    }

    // Create a command encoder/recorder
    auto commandRecorder = _device.createCommandRecorder();

    // Update Camera UBO data
    static float angle = 0.0f;
    angle += 0.1f;
    if (angle > 360.0f)
        angle -= 360.0f;

    auto cameraBufferData = _cameraUBOBuffer.map();
    glm::mat4 cameraMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
    std::memcpy(cameraBufferData, glm::value_ptr(cameraMatrix), 16 * sizeof(float));
    _cameraUBOBuffer.unmap();

    // Begin render pass
    RenderPassCommandRecorder opaquePass = commandRecorder.beginRenderPass(
            RenderPassCommandRecorderOptions{
                    .colorAttachments = {
                            {
                                    .view = _swapchainViews.at(currentImageIndex),
                                    .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f },
                                    .finalLayout = TextureLayout::PresentSrc,
                            },
                    },
                    .depthStencilAttachment = {
                            .view = _depthTextureView,
                    },
            });

    // Bind pipeline
    opaquePass.setPipeline(_pipeline.handle());

    // Bind vertex buffer
    opaquePass.setVertexBuffer(0, _vertexBuffer.handle());

    // Binding GPU Resources (UBO / SSBO / Textures)
    opaquePass.setBindGroup(0, _bindGroup);

    // Issue draw command
    const DrawCommand drawCmd = { .vertexCount = 3 };
    opaquePass.draw(drawCmd);

    // End render pass
    opaquePass.end();

    // End recording
    const CommandBuffer commands = commandRecorder.finish();

    // Submit command buffer to queue
    // - wait for the imageAvailableSemaphore
    // - will signal the renderCompleteSemaphore when execution on the GPU is completed
    // - will signal the frameInFlightFence so that we can wait on the CPU for execution to have completed
    _queue.submit(SubmitOptions{
            .commandBuffers = { commands },
            .waitSemaphores = { _imageAvailableSemaphore },
            .signalSemaphores = { _renderCompleteSemaphore },
            .signalFence = _frameInFlightFence,
    });

    // Present and request next frame (need API for this)
    // - wait for the renderCompleteSemaphore to have been signalled as we only want to present once
    //   everything has been rendered
    _queue.present(PresentOptions{
            .waitSemaphores = { _renderCompleteSemaphore },
            .swapchainInfos = { { .swapchain = _swapchain, .imageIndex = currentImageIndex } },
    });

    // Wait for frame to have completed its execution
    _frameInFlightFence.wait();
}

void KDGpuRenderer::resize(uint32_t width, uint32_t height)
{
    _width = width;
    _height = height;
    createSwapchain();
}

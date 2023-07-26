/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/buffer_options.h>
#include <KDGpu/bind_group_layout_options.h>
#include <KDGpu/bind_group_layout.h>
#include <KDGpu/bind_group_options.h>
#include <KDGpu/device.h>
#include <KDGpu/instance.h>
#include <KDGpu/graphics_pipeline.h>
#include <KDGpu/graphics_pipeline_options.h>
#include <KDGpu/fence.h>
#include <KDGpu/gpu_core.h>
#include <KDGpu/gpu_semaphore.h>
#include <KDGpu/queue.h>
#include <KDGpu/render_pass_command_recorder_options.h>
#include <KDGpu/swapchain.h>
#include <KDGpu/swapchain_options.h>
#include <KDGpu/texture.h>
#include <KDGpu/texture_options.h>
#include <KDGpu/utils/formatters.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

#include <KDGui/gui_application.h>
#include <KDGui/window.h>

#include <KDUtils/file.h>
#include <KDUtils/dir.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <KDFoundation/config.h> // For KD_PLATFORM
#if defined(KD_PLATFORM_WIN32)
#include <KDGui/platform/win32/win32_platform_window.h>
#endif
#if defined(KD_PLATFORM_LINUX)
#include <KDGui/platform/linux/xcb/linux_xcb_platform_window.h>
#endif
#if defined(KD_PLATFORM_MACOS)
extern CAMetalLayer *createMetalLayer(KDGui::Window *window);
#endif

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <map>
#include <memory>
#include <span>
#include <vector>

using namespace KDGui;
using namespace KDGpu;

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

namespace {

std::vector<uint32_t> readShaderFile(const std::string &filename)
{
    using namespace KDUtils;

    File file(File::exists(filename) ? filename : Dir::applicationDir().absoluteFilePath(filename));

    if (!file.open(std::ios::in | std::ios::binary)) {
        SPDLOG_CRITICAL("Failed to open file {}", filename);
        throw std::runtime_error("Failed to open file");
    }

    const ByteArray fileContent = file.readAll();
    std::vector<uint32_t> buffer(fileContent.size() / 4);
    std::memcpy(buffer.data(), fileContent.data(), fileContent.size());

    return buffer;
}

} // namespace

int main()
{
    //![0]
    GuiApplication app;
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();

    // Request an instance of the api with whatever layers and extensions we wish to request.
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "hello_triangle_native",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0),
    });

    // Create a window and platform surface from it suitable for use with our chosen graphics API.
    Window window;
    window.title = "hello_triangle_native";
    window.width = 1920;
    window.height = 1080;
    window.visible = true;
    window.visible.valueChanged().connect([&app](bool visible) {
        if (visible == false)
            app.quit();
    });
    //![0]

    //![2]
#if defined(KD_PLATFORM_WIN32)
    auto win32Window = dynamic_cast<Win32PlatformWindow *>(window.platformWindow());
    SurfaceOptions surfaceOptions = {
        .hWnd = win32Window->handle()
    };
#endif

#if defined(KD_PLATFORM_LINUX)
    auto xcbWindow = dynamic_cast<LinuxXcbPlatformWindow *>(window.platformWindow());
    SurfaceOptions surfaceOptions = {
        .connection = xcbWindow->connection(),
        .window = xcbWindow->handle()
    };
#endif

#if defined(KD_PLATFORM_MACOS)
    SurfaceOptions surfaceOptions = {
        .layer = createMetalLayer(&window)
    };
#endif

    Surface surface = instance.createSurface(surfaceOptions);
    //![2]

    //![4]
    // Enumerate the adapters (physical devices) and select one to use. Here we look for
    // a discrete GPU. In a real app, we could fallback to an integrated one.
    Adapter *selectedAdapter = instance.selectAdapter(AdapterDeviceType::Default);
    //![4]

    if (!selectedAdapter) {
        SPDLOG_WARN("Unable to find a discrete GPU. Aborting...");
        return -1;
    }

    //![5]
    // We can easily query the adapter for various features, properties and limits.
    SPDLOG_WARN("maxBoundDescriptorSets = {}", selectedAdapter->properties().limits.maxBoundDescriptorSets);
    SPDLOG_WARN("multiDrawIndirect = {}", selectedAdapter->features().multiDrawIndirect);
    //![5]

    //![6]
    auto queueTypes = selectedAdapter->queueTypes();
    const bool hasGraphicsAndCompute = queueTypes[0].supportsFeature(QueueFlags(QueueFlagBits::GraphicsBit) | QueueFlags(QueueFlagBits::ComputeBit));
    SPDLOG_WARN("Queue family 0 graphics and compute support: {}", hasGraphicsAndCompute);
    //![6]

    //![7]
    // We are now able to query the adapter for swapchain properties and presentation support with the window surface
    const auto swapchainProperties = selectedAdapter->swapchainProperties(surface);
    const bool supportsPresentation = selectedAdapter->supportsPresentation(surface, 0); // Query about the 1st queue type
    SPDLOG_WARN("Queue family 0 supports presentation: {}", supportsPresentation);

    if (!supportsPresentation || !hasGraphicsAndCompute) {
        SPDLOG_WARN("Selected adapter queue family 0 does not meet requirements. Aborting.");
        return -1;
    }
    //![7]

    //![8]
    // Now we can create a device from the selected adapter that we can then use to interact with the GPU.
    Device device = selectedAdapter->createDevice();
    Queue queue = device.queues()[0];

    Swapchain swapchain;
    std::vector<TextureView> swapchainViews;
    Texture depthTexture;
    TextureView depthTextureView;

    Format swapchainFormat;
    Format depthTextureFormat;

    auto createSwapchain = [&] {
        const AdapterSwapchainProperties swapchainProperties = device.adapter()->swapchainProperties(surface);

        // Create a swapchain of images that we will render to.
        const SwapchainOptions swapchainOptions = {
            .surface = surface.handle(),
            .minImageCount = getSuitableImageCount(swapchainProperties.capabilities),
            .imageExtent = { .width = window.width(), .height = window.height() },
            .oldSwapchain = swapchain,
        };

        swapchain = device.createSwapchain(swapchainOptions);
        const auto &swapchainTextures = swapchain.textures();
        const auto swapchainTextureCount = swapchainTextures.size();

        swapchainViews.clear();
        swapchainViews.reserve(swapchainTextureCount);
        for (uint32_t i = 0; i < swapchainTextureCount; ++i) {
            auto view = swapchainTextures[i].createView({ .format = swapchainOptions.format });
            swapchainViews.push_back(std::move(view));
        }

        //![3]
        // Create a depth texture to use for rendering
        const TextureOptions depthTextureOptions = {
            .type = TextureType::TextureType2D,
            .format = Format::D24_UNORM_S8_UINT,
            .extent = { window.width(), window.height(), 1 },
            .mipLevels = 1,
            .usage = TextureUsageFlagBits::DepthStencilAttachmentBit,
            .memoryUsage = MemoryUsage::GpuOnly
        };
        //![3]
        depthTexture = device.createTexture(depthTextureOptions);
        depthTextureView = depthTexture.createView();

        swapchainFormat = swapchainOptions.format;
        depthTextureFormat = depthTextureOptions.format;
    };

    createSwapchain();
    //![8]

    // Create a buffer to hold triangle vertex data
    Buffer vertexBuffer = device.createBuffer(BufferOptions{
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
        void *bufferData = vertexBuffer.map();
        std::memcpy(bufferData, vertexData.data(), vertexData.size() * sizeof(float));
        vertexBuffer.unmap();
    }

    Buffer cameraUBOBuffer = device.createBuffer(BufferOptions{
            .size = 16 * sizeof(float), // 1 * mat4x4
            .usage = BufferUsageFlagBits::UniformBufferBit,
            .memoryUsage = MemoryUsage::CpuToGpu, // So we can map it to CPU address space
    });
    {
        void *bufferData = cameraUBOBuffer.map();
        glm::mat4 m(1.0);
        std::memcpy(bufferData, &m, 16 * sizeof(float));
        cameraUBOBuffer.unmap();
    }

    // Create a vertex shader and fragment shader (spir-v only for now)
    const auto vertexShaderPath = KDGpu::assetPath() + "/shaders/examples/hello_triangle_native/hello_triangle.vert.spv";
    ShaderModule vertexShader = device.createShaderModule(readShaderFile(vertexShaderPath));

    const auto fragmentShaderPath = KDGpu::assetPath() + "/shaders/examples/hello_triangle_native/hello_triangle.frag.spv";
    ShaderModule fragmentShader = device.createShaderModule(readShaderFile(fragmentShaderPath));

    BindGroupLayout bindGroupLayout = device.createBindGroupLayout(BindGroupLayoutOptions{
            .bindings = {
                    { // Camera uniforms
                      .binding = 0,
                      .count = 1,
                      .resourceType = ResourceBindingType::UniformBuffer,
                      .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit) },
            },
    });

    // Create a pipeline layout (array of bind group layouts)
    PipelineLayout pipelineLayout = device.createPipelineLayout(PipelineLayoutOptions{
            .bindGroupLayouts = { bindGroupLayout },
    });

    // Create a pipeline
    GraphicsPipeline pipeline = device.createGraphicsPipeline(GraphicsPipelineOptions{
            .shaderStages = {
                    { .shaderModule = vertexShader.handle(), .stage = ShaderStageFlagBits::VertexBit },
                    { .shaderModule = fragmentShader.handle(), .stage = ShaderStageFlagBits::FragmentBit },
            },
            .layout = pipelineLayout.handle(),
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
                    { .format = swapchainFormat },
            },
            .depthStencil = {
                    .format = depthTextureFormat,
                    .depthWritesEnabled = true,
                    .depthCompareOperation = CompareOperation::Less,
            },
    });

    // Implement the render loop
    // Most of the render pass is the same between frames. The only thing that changes, is which image
    // of the swapchain we wish to render to. So set up what we can here, and in the render loop we will
    // just update the color texture view.

    BindGroup bindGroup = device.createBindGroup(BindGroupOptions{
            .layout = bindGroupLayout,
            .resources = {
                    {
                            .binding = 0,
                            .resource = UniformBufferBinding{ .buffer = cameraUBOBuffer },
                    },
            },
    });

    // Update BindGroup for binding 0
    bindGroup.update(BindGroupEntry{ .binding = 0, .resource = UniformBufferBinding{ .buffer = cameraUBOBuffer } });

    //![9]
    const GpuSemaphore imageAvailableSemaphore = device.createGpuSemaphore();
    const GpuSemaphore renderCompleteSemaphore = device.createGpuSemaphore();
    //![15]
    Fence frameInFlightFence = device.createFence(FenceOptions{ .createSignalled = true });
    //![15]
    //![9]

    //![10]
    while (window.visible()) {
        //![10]
        // Reset fence
        //![17]
        frameInFlightFence.reset();
        //![17]

        // Acquire next swapchain image
        //![11]
        uint32_t currentImageIndex = 0;
        //![12]
        AcquireImageResult result = swapchain.getNextImageIndex(currentImageIndex, imageAvailableSemaphore);
        //![12]
        if (result == AcquireImageResult::OutOfDate) {
            // This can happen when swapchain was resized
            // We need to recreate the swapchain and retry
            createSwapchain();
            result = swapchain.getNextImageIndex(currentImageIndex, imageAvailableSemaphore);
        }
        //![11]
        if (result != AcquireImageResult::Success) {
            SPDLOG_LOGGER_ERROR(Logger::logger(), "Unable to acquire swapchain image");
        }

        // Create a command encoder/recorder
        auto commandRecorder = device.createCommandRecorder();

        // Note: with Vulkan we can't perform Buffer updates during a RenderPass (not sure about other APIs)

        // Update Camera UBO data
        static float angle = 0.0f;
        angle += 0.1f;
        if (angle > 360.0f)
            angle -= 360.0f;

        auto cameraBufferData = cameraUBOBuffer.map();
        glm::mat4 cameraMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
        std::memcpy(cameraBufferData, glm::value_ptr(cameraMatrix), 16 * sizeof(float));
        cameraUBOBuffer.unmap();

        // Begin render pass
        RenderPassCommandRecorder opaquePass = commandRecorder.beginRenderPass(
                RenderPassCommandRecorderOptions{
                        .colorAttachments = {
                                {
                                        .view = swapchainViews.at(currentImageIndex),
                                        .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f },
                                        .finalLayout = TextureLayout::PresentSrc,
                                },
                        },
                        .depthStencilAttachment = {
                                .view = depthTextureView,
                        },
                });

        // Bind pipeline
        opaquePass.setPipeline(pipeline.handle());

        // Bind vertex buffer
        opaquePass.setVertexBuffer(0, vertexBuffer.handle());

        // Binding GPU Resources (UBO / SSBO / Textures)
        opaquePass.setBindGroup(0, bindGroup);

        // Bind any resources (none needed for hello_triangle)

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
        //![13]
        queue.submit(SubmitOptions{
                .commandBuffers = { commands },
                .waitSemaphores = { imageAvailableSemaphore },
                .signalSemaphores = { renderCompleteSemaphore },
                //![16]
                .signalFence = frameInFlightFence,
                //![16]
        });
        //![13]

        // Present and request next frame (need API for this)
        // - wait for the renderCompleteSemaphore to have been signalled as we only want to present once
        //   everything has been rendered
        //![14]
        queue.present(PresentOptions{
                .waitSemaphores = { renderCompleteSemaphore },
                .swapchainInfos = { { .swapchain = swapchain, .imageIndex = currentImageIndex } },
        });
        //![14]

        // Wait for frame to have completed its execution
        //![18]
        frameInFlightFence.wait();
        //![18]

        // Process application events
        app.processEvents();
    }

    return app.exec();
}

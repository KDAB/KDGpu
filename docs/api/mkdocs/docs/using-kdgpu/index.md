# Using KDGpu

## Setting up and application class and a Windows

We use KDGui to conveniently create an Application and a Window and keep the code below
streamlined.

    #include <KDGui/gui_application.h>
    #include <KDGui/window.h>

    using namespace KDGui;
    using namespace KDGpu;
    ...

    GuiApplication app;

    Window window;
    window.width = 1920;
    window.height = 1080;
    window.visible = true;


## Selecting a Rendering API

At the moment, KDGpu only supports Vulkan:

    #include <KDGpu/vulkan/vulkan_graphics_api.h>

    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();


## Instance

    #include <KDGpu/instance.h>

    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "MyApplication",
            .applicationVersion = SERENITY_MAKE_API_VERSION(0, 1, 0, 0),
    });

## Surface

Since we are using KDGui, we can leverage KDGpuKDGui to simplify the Surface creation.

    #include <KDGpu/surface.h>
    #include <KDGpuKDGui/view.h>

    const SurfaceOptions surfaceOptions = KDGpuKDGui::surfaceOptions(&window);
    Surface surface = instance.createSurface(surfaceOptions);


## Physical Device Selection and Device Creation

    #include <KDGpu/device.h>
    #include <KDGpu/queue.h>

    // Select an appropriate Physical Device
    Adapter *selectedAdapter = instance.selectAdapter(AdapterDeviceType::Default);

    Device device = selectedAdapter->createDevice();

### Retrieve Queue

    Queue queue = device.queues()[0];

## GPU Resources

### Buffer

#### Creation

    Buffer buffer = device.createBuffer(BufferOptions {
        .size = 3 * 2 * 4 * sizeof(float), // 3 vertices * 2 attributes * 4 float components
        .usage = BufferUsageFlags(BufferUsageFlagBits::VertexBufferBit),
        .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
    });
    Buffer cameraUBOBuffer = device.createBuffer(BufferOptions{
            .size = 16 * sizeof(float), // 1 * mat4x4
            .usage = BufferUsageFlagBits::UniformBufferBit,
            .memoryUsage = MemoryUsage::CpuToGpu, // So we can map it to CPU address space
    });

#### Data Upload

    {
        const std::vector<float> vertexData = {
        1.0f, -1.0f, 0.0f, 1.0f, // position
        1.0f,  0.0f, 0.0f, 1.0f, // color
        -1.0f, -1.0f, 0.0f, 1.0f, // position
        0.0f,  1.0f, 0.0f, 1.0f, // color
        0.0f,  1.0f, 0.0f, 1.0f, // position
        0.0f,  0.0f, 1.0f, 1.0f, // color
        };
        auto bufferData = buffer.map();
        std::memcpy(bufferData, vertexData.data(), vertexData.size() * sizeof(float));
        buffer.unmap();
    }

    {
        void *bufferData = cameraUBOBuffer.map();
        glm::mat4 m(1.0);
        std::memcpy(bufferData, &m, 16 * sizeof(float));
        cameraUBOBuffer.unmap();
    }

### Texture

    #include <KDGpu/texture.h>
    #include <KDGpu/texture_options.h>

    Texture depthTexture = device.createTexture(TextureOptions {
        .type = TextureType::TextureType2D,
        .format = Format::D24_UNORM_S8_UINT,
        .extent = { window.width(), window.height(), 1 },
        .mipLevels = 1,
        .usage = TextureUsageFlagBits::DepthStencilAttachmentBit,
        .memoryUsage = MemoryUsage::GpuOnly
    });

    TextureView depthTextureView = depthTexture.createView();

## Swapchain

    #include <KDGpu/swapchain.h>
    #include <KDGpu/swapchain_options.h>

    Swapchain swapchain = device.createSwapchain(SwapchainOptions {
        .surface = surface.handle(),
        .imageExtent = { .width = window.width(), .height = window.height() },
    });

    // Retrieve Textures
    const auto &swapchainTextures = swapchain.textures();
    const auto swapchainTextureCount = swapchainTextures.size();

    // Create Texture Views for each Swapchain image
    std::vector<TextureView> swapchainViews;
    for (uint32_t i = 0; i < swapchainTextureCount; ++i) {
        auto view = swapchainTextures[i].createView({ .format = swapchainOptions.format });
        swapchainViews.push_back(std::move(view));
    }

## Pipeline Creation

### Shader Modules

    // Create a vertex shader and fragment shader (spir-v only for now)
    const auto vertexShaderPath = KDGpu::assetPath() + "/shaders/hello_triangle.vert.spv";
    ShaderModule vertexShader = device.createShaderModule(KDGpu::readShaderFile(vertexShaderPath));

    const auto fragmentShaderPath = KDGpu::assetPath() + "/shaders/hello_triangle.frag.spv";
    ShaderModule fragmentShader = device.createShaderModule(KDGpu::readShaderFile(fragmentShaderPath));

### Bind Group Layout

    #include <KDGpu/bind_group_layout_options.h>
    #include <KDGpu/bind_group_layout.h>

    BindGroupLayout bindGroupLayout = device.createBindGroupLayout(BindGroupLayoutOptions{
            .bindings = {
                    { // Camera uniforms
                      .binding = 0,
                      .count = 1,
                      .resourceType = ResourceBindingType::UniformBuffer,
                      .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit) },
            },
    });

### Pipeline Layout

    PipelineLayout pipelineLayout = device.createPipelineLayout(PipelineLayoutOptions{
            .bindGroupLayouts = { bindGroupLayout },
    });

### Bind Group

    BindGroup bindGroup = device.createBindGroup(BindGroupOptions{
            .layout = bindGroupLayout,
            .resources = {
                    {
                            .binding = 0,
                            .resource = UniformBufferBinding{ .buffer = cameraUBOBuffer },
                    },
            },
    });

### Graphics Pipeline

    #include <KDGpu/graphics_pipeline.h>
    #include <KDGpu/graphics_pipeline_options.h>

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
                    { .format = swapchainOptions.format },
            },
            .depthStencil = {
                    .format = Format::D24_UNORM_S8_UINT,
                    .depthWritesEnabled = true,
                    .depthCompareOperation = CompareOperation::Less,
            },
    });

## Command Recording

    #include <KDGpu/render_pass_command_recorder_options.h>
    #include <KDGpu/gpu_semaphore.h>

    const GpuSemaphore renderCompleteSemaphore = device.createGpuSemaphore();

    // Acquire next swapchain image
    uint32_t currentImageIndex = 0;
    AcquireImageResult result = swapchain.getNextImageIndex(currentImageIndex, imageAvailableSemaphore);

    // Update GPU resources
    static float angle = 0.0f;
    angle += 0.1f;

    auto cameraBufferData = cameraUBOBuffer.map();
    glm::mat4 cameraMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
    std::memcpy(cameraBufferData, glm::value_ptr(cameraMatrix), 16 * sizeof(float));
    cameraUBOBuffer.unmap();


    CommandRecorder commandRecorder = device.createCommandRecorder();

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

    // Issue draw command
    const DrawCommand drawCmd = { .vertexCount = 3 };
    opaquePass.draw(drawCmd);

    // End render pass
    opaquePass.end();

    // End recording
    const CommandBuffer commands = commandRecorder.finish();

## Queue Submission

        queue.submit(SubmitOptions{
            .commandBuffers = { commands },
            .signalSemaphores = { renderCompleteSemaphore },
        });

## Presentation

        // Present and request next frame (need API for this)
        // - wait for the renderCompleteSemaphore to have been signalled as we only want to present once
        //   everything has been rendered
        queue.present(PresentOptions{
                .waitSemaphores = { renderCompleteSemaphore },
                .swapchainInfos = { { .swapchain = swapchain, .imageIndex = currentImageIndex } },
        });

        queue.waitIdle();

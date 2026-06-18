/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2026 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/graphics_pipeline.h>
#include <KDGpu/graphics_pipeline_options.h>
#include <KDGpu/dynamic_render_pass_command_recorder.h>
#include <KDGpu/command_recorder.h>
#include <KDGpu/device.h>
#include <KDGpu/queue.h>
#include <KDGpu/instance.h>
#include <KDGpu/texture.h>
#include <KDGpu/render_pass.h>
#include <KDGpu/render_pass_options.h>
#include <KDGpu/texture_options.h>
#include <KDGpu/texture_view.h>
#include <KDGpu/device_options.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>
#include <KDGpu/buffer.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/bind_group.h>
#include <KDGpu/bind_group_layout.h>
#include <KDGpu/bind_group_layout_options.h>
#include <KDGpu/bind_group_options.h>
#include <KDGpu/sampler.h>
#include <KDGpu/pipeline_layout_options.h>

#include <KDUtils/file.h>
#include <KDUtils/dir.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDGpu;

namespace {
inline std::string assetPath()
{
#if defined(KDGPU_ASSET_PATH)
    return KDGPU_ASSET_PATH;
#else
    return "";
#endif
}

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

TEST_SUITE("DynamicDynamicRenderPassCommandRecorder")
{
    // GIVEN
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "DynamicRenderPassCommandRecorder",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });

    Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);
    const bool hasDynamicRenderingSupport = discreteGPUAdapter->features().dynamicRendering;
    const bool hasSubpassDynamicRenderingSupport = discreteGPUAdapter->features().dynamicRenderingLocalRead;
    const bool hasMultiviewSupport = discreteGPUAdapter->features().multiView;

    TEST_CASE("DynamicDynamicRenderPassCommandRecorder" * doctest::skip(!hasDynamicRenderingSupport))
    {
        // GIVEN
        Device device = discreteGPUAdapter->createDevice(DeviceOptions{
                .requestedFeatures{
                        .dynamicRendering = true,
                },
        });

        // Bind group layout
        const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(BindGroupLayoutOptions{
                .bindings = {},
        });

        // Pipeline layout with the bind group layout
        const PipelineLayout pipelineLayout = device.createPipelineLayout(PipelineLayoutOptions{
                .bindGroupLayouts = { bindGroupLayout },
        });

        // Pipeline that uses dynamic rendering
        const auto vertexShaderPath = assetPath() + "/shaders/tests/render_pass_command_recorder/triangle.vert.spv";
        auto vertexShader = device.createShaderModule(readShaderFile(vertexShaderPath));

        const auto fragmentShaderPath = assetPath() + "/shaders/tests/render_pass_command_recorder/triangle.frag.spv";
        auto fragmentShader = device.createShaderModule(readShaderFile(fragmentShaderPath));

        const GraphicsPipeline pipeline = device.createGraphicsPipeline(GraphicsPipelineOptions{
                .shaderStages = {
                        { .shaderModule = vertexShader.handle(), .stage = ShaderStageFlagBits::VertexBit },
                        { .shaderModule = fragmentShader.handle(), .stage = ShaderStageFlagBits::FragmentBit },
                },
                .layout = pipelineLayout.handle(),
                .vertex = {
                        .buffers = {
                                { .binding = 0, .stride = 6 * sizeof(float) },
                        },
                        .attributes = {
                                { .location = 0, .binding = 0, .format = Format::R32G32B32A32_SFLOAT }, // Position
                                { .location = 1, .binding = 0, .format = Format::R32G32B32A32_SFLOAT, .offset = 3 * sizeof(float) }, // Color
                        },
                },
                .renderTargets = {
                        { .format = Format::R8G8B8A8_UNORM },
                },
                .depthStencil = { .format = Format::D24_UNORM_S8_UINT, .depthWritesEnabled = true, .depthCompareOperation = CompareOperation::Less },
                .dynamicRendering = {
                        .enabled = true,
                },
        });

        // THEN
        CHECK(device.isValid());
        CHECK(vertexShader.isValid());
        CHECK(fragmentShader.isValid());
        CHECK(bindGroupLayout.isValid());
        CHECK(pipelineLayout.isValid());
        CHECK(pipeline.isValid());

        // Texture Attachments
        const Texture colorTexture = device.createTexture(TextureOptions{
                .type = TextureType::TextureType2D,
                .format = Format::R8G8B8A8_UNORM,
                .extent = { 256, 256, 1 },
                .mipLevels = 1,
                .samples = SampleCountFlagBits::Samples1Bit,
                .usage = TextureUsageFlagBits::ColorAttachmentBit,
                .memoryUsage = MemoryUsage::GpuOnly,
        });
        const Texture depthTexture = device.createTexture(TextureOptions{
                .type = TextureType::TextureType2D,
#if defined(KD_PLATFORM_MACOS)
                .format = Format::D32_SFLOAT_S8_UINT,
#else
                .format = Format::D24_UNORM_S8_UINT,
#endif
                .extent = { 256, 256, 1 },
                .mipLevels = 1,
                .samples = SampleCountFlagBits::Samples1Bit,
                .usage = TextureUsageFlagBits::DepthStencilAttachmentBit,
                .memoryUsage = MemoryUsage::GpuOnly,
        });

        const TextureView colorTextureView = colorTexture.createView();
        const TextureView depthTextureView = depthTexture.createView();

        // THEN
        CHECK(colorTextureView.isValid());
        CHECK(depthTextureView.isValid());

        SUBCASE("Can't be default constructed")
        {
            // EXPECT
            REQUIRE(!std::is_default_constructible<DynamicRenderPassCommandRecorder>::value);
            REQUIRE(!std::is_trivially_default_constructible<DynamicRenderPassCommandRecorder>::value);
        }

        SUBCASE("A constructed DynamicRenderPassCommandRecorder from a Vulkan API")
        {
            // GIVEN
            CommandRecorder commandRecorder = device.createCommandRecorder();

            // WHEN -> Creating Dynamic Rendering DynamicRenderPassCommandRecorder
            const RenderPassCommandRecorderWithDynamicRenderingOptions dynamicRenderingOptions{
                .colorAttachments = {
                        {
                                .view = colorTextureView,
                                .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f },
                                .finalLayout = TextureLayout::PresentSrc,
                        },
                },
                .depthStencilAttachment = {
                        .view = depthTextureView,
                }
            };

            DynamicRenderPassCommandRecorder renderPassRecorder = commandRecorder.beginRenderPass(dynamicRenderingOptions);

            // THEN -> Test can use dynamic rendering pipeline
            renderPassRecorder.setPipeline(pipeline);

            renderPassRecorder.end();
            CommandBuffer commandBuffer = commandRecorder.finish();

            // THEN
            CHECK(commandRecorder.isValid());
            CHECK(renderPassRecorder.isValid());
            CHECK(commandBuffer.isValid());
        }

        SUBCASE("Move constructor & move assignment")
        {
            // GIVEN
            CommandRecorder commandRecorder = device.createCommandRecorder();
            const RenderPassCommandRecorderWithDynamicRenderingOptions dynamicRenderPassOptions{
                .colorAttachments = {
                        { .view = colorTextureView,
                          .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f },
                          .finalLayout = TextureLayout::PresentSrc } },
                .depthStencilAttachment = {
                        .view = depthTextureView,
                }
            };

            // WHEN
            DynamicRenderPassCommandRecorder renderPassRecorder1 = commandRecorder.beginRenderPass(dynamicRenderPassOptions);
            DynamicRenderPassCommandRecorder renderPassRecorder2(std::move(renderPassRecorder1));
            renderPassRecorder2.end();

            // THEN
            CHECK(!renderPassRecorder1.isValid());
            CHECK(renderPassRecorder2.isValid());

            // WHEN
            DynamicRenderPassCommandRecorder renderPassRecorder3 = commandRecorder.beginRenderPass(dynamicRenderPassOptions);
            renderPassRecorder3.end();

            const auto renderPassRecorder2Handle = renderPassRecorder2.handle();
            renderPassRecorder3 = std::move(renderPassRecorder2);

            // THEN
            CHECK(!renderPassRecorder2.isValid());
            CHECK(renderPassRecorder3.isValid());
            CHECK(renderPassRecorder3.handle() == renderPassRecorder2Handle);
        }

        SUBCASE("Destruction")
        {
            // GIVEN
            const RenderPassCommandRecorderWithDynamicRenderingOptions dynamicRenderPassOptions{
                .colorAttachments = {
                        { .view = colorTextureView,
                          .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f },
                          .finalLayout = TextureLayout::PresentSrc } },
                .depthStencilAttachment = {
                        .view = depthTextureView,
                }
            };
            Handle<DynamicRenderPassCommandRecorder_t> recorderHandle;

            {
                // WHEN
                CommandRecorder commandRecorder = device.createCommandRecorder();
                DynamicRenderPassCommandRecorder renderPassRecorder = commandRecorder.beginRenderPass(dynamicRenderPassOptions);
                recorderHandle = renderPassRecorder.handle();
                renderPassRecorder.end();

                CommandBuffer commandBuffer = commandRecorder.finish();

                // THEN
                CHECK(commandRecorder.isValid());
                CHECK(commandBuffer.isValid());
                CHECK(renderPassRecorder.isValid());
                CHECK(recorderHandle.isValid());
                CHECK(api->resourceManager()->getDynamicRenderPassCommandRecorder(recorderHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getDynamicRenderPassCommandRecorder(recorderHandle) == nullptr);
        }
    }

    TEST_CASE("DynamicRenderPassCommandRecorder - MultiView" * doctest::skip(!hasDynamicRenderingSupport || !hasMultiviewSupport))
    {
        REQUIRE(discreteGPUAdapter->properties().multiViewProperties.maxMultiViewCount > 1);
        REQUIRE(discreteGPUAdapter->features().multiView);

        // GIVEN
        Device device = discreteGPUAdapter->createDevice(DeviceOptions{
                .requestedFeatures = {
                        .sampleRateShading = true,
                        .multiView = true,
                        .dynamicRendering = true,
                },
        });

        const auto vertexShaderPath = assetPath() + "/shaders/tests/render_pass_command_recorder/triangle-multi-view.vert.spv";
        auto vertexShader = device.createShaderModule(readShaderFile(vertexShaderPath));

        const auto fragmentShaderPath = assetPath() + "/shaders/tests/render_pass_command_recorder/triangle-multi-view.frag.spv";
        auto fragmentShader = device.createShaderModule(readShaderFile(fragmentShaderPath));

        const Texture colorTexture = device.createTexture(TextureOptions{
                .type = TextureType::TextureType2D,
                .format = Format::R8G8B8A8_UNORM,
                .extent = { 256, 256, 1 },
                .mipLevels = 1,
                .arrayLayers = 2,
                .samples = SampleCountFlagBits::Samples1Bit,
                .usage = TextureUsageFlagBits::ColorAttachmentBit,
                .memoryUsage = MemoryUsage::GpuOnly,
        });
        const Texture depthTexture = device.createTexture(TextureOptions{
                .type = TextureType::TextureType2D,
#if defined(KD_PLATFORM_MACOS)
                .format = Format::D32_SFLOAT_S8_UINT,
#else
                .format = Format::D24_UNORM_S8_UINT,
#endif
                .extent = { 256, 256, 1 },
                .mipLevels = 1,
                .arrayLayers = 2,
                .samples = SampleCountFlagBits::Samples1Bit,
                .usage = TextureUsageFlagBits::DepthStencilAttachmentBit,
                .memoryUsage = MemoryUsage::GpuOnly,
        });

        const TextureView colorTextureView = colorTexture.createView(TextureViewOptions{
                .viewType = ViewType::ViewType2DArray,
                .range = {
                        .aspectMask = TextureAspectFlagBits::ColorBit,
                        .layerCount = 2,
                },
        });
        const TextureView depthTextureView = depthTexture.createView(TextureViewOptions{
                .viewType = ViewType::ViewType2DArray,
                .range = {
                        .aspectMask = (TextureAspectFlagBits::DepthBit | TextureAspectFlagBits::StencilBit),
                        .layerCount = 2,
                },
        });

        const PipelineLayout pipelineLayout = device.createPipelineLayout();
        const GraphicsPipeline pipeline = device.createGraphicsPipeline(GraphicsPipelineOptions{
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
                        { .format = Format::R8G8B8A8_UNORM },
                },
                .depthStencil = { .format = Format::D24_UNORM_S8_UINT, .depthWritesEnabled = true, .depthCompareOperation = CompareOperation::Less },
                .viewCount = 2,
        });

        // THEN
        REQUIRE(pipelineLayout.isValid());
        REQUIRE(pipeline.isValid());
        REQUIRE(colorTextureView.isValid());
        REQUIRE(depthTextureView.isValid());
        REQUIRE(device.isValid());

        SUBCASE("A constructed DynamicRenderPassCommandRecorder with MultiView from a Vulkan API")
        {
            // GIVEN
            CommandRecorder commandRecorder = device.createCommandRecorder();
            const RenderPassCommandRecorderWithDynamicRenderingOptions renderPassOptions{
                .colorAttachments = {
                        { .view = colorTextureView,
                          .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f },
                          .finalLayout = TextureLayout::PresentSrc } },
                .depthStencilAttachment = {
                        .view = depthTextureView,
                },
                .viewCount = 2,
            };

            // WHEN
            DynamicRenderPassCommandRecorder renderPassRecorder = commandRecorder.beginRenderPass(renderPassOptions);
            renderPassRecorder.setPipeline(pipeline);
            renderPassRecorder.end();

            CommandBuffer commandBuffer = commandRecorder.finish();

            // THEN
            CHECK(commandRecorder.isValid());
            CHECK(renderPassRecorder.isValid());
        }
    }

    TEST_CASE("DynamicRenderPassCommandRecorder - Resolve MSAA Color & Depth" * doctest::skip(!hasDynamicRenderingSupport))
    {
        const bool supportsAverageDepthResolveMode = discreteGPUAdapter->properties().depthResolveProperties.supportedDepthResolveModes.testFlag(ResolveModeFlagBits::Average);
        if (!supportsAverageDepthResolveMode)
            return;

        const SampleCountFlagBits samples = SampleCountFlagBits::Samples4Bit;
        const bool colorFrameBufferSupportsSamples = discreteGPUAdapter->properties().limits.framebufferColorSampleCounts.testFlag(samples);
        const bool depthFrameBufferSupportsSamples = discreteGPUAdapter->properties().limits.framebufferDepthSampleCounts.testFlag(samples);

        REQUIRE(discreteGPUAdapter->features().sampleRateShading);
        REQUIRE(supportsAverageDepthResolveMode);
        REQUIRE(colorFrameBufferSupportsSamples);
        REQUIRE(depthFrameBufferSupportsSamples);

        // GIVEN
        Device device = discreteGPUAdapter->createDevice(DeviceOptions{
                .requestedFeatures = {
                        .sampleRateShading = true,
                        .dynamicRendering = true,
                },
        });

        const auto vertexShaderPath = assetPath() + "/shaders/tests/render_pass_command_recorder/triangle.vert.spv";
        auto vertexShader = device.createShaderModule(readShaderFile(vertexShaderPath));

        const auto fragmentShaderPath = assetPath() + "/shaders/tests/render_pass_command_recorder/triangle.frag.spv";
        auto fragmentShader = device.createShaderModule(readShaderFile(fragmentShaderPath));

        const Texture colorMSAATexture = device.createTexture(TextureOptions{
                .type = TextureType::TextureType2D,
                .format = Format::R8G8B8A8_UNORM,
                .extent = { 256, 256, 1 },
                .mipLevels = 1,
                .samples = samples,
                .usage = TextureUsageFlagBits::ColorAttachmentBit,
                .memoryUsage = MemoryUsage::GpuOnly,
        });
        const Texture depthMSAATexture = device.createTexture(TextureOptions{
                .type = TextureType::TextureType2D,
#if defined(KD_PLATFORM_MACOS)
                .format = Format::D32_SFLOAT_S8_UINT,
#else
                .format = Format::D24_UNORM_S8_UINT,
#endif
                .extent = { 256, 256, 1 },
                .mipLevels = 1,
                .samples = samples,
                .usage = TextureUsageFlagBits::DepthStencilAttachmentBit,
                .memoryUsage = MemoryUsage::GpuOnly,
        });

        const Texture colorResolveTexture = device.createTexture(TextureOptions{
                .type = TextureType::TextureType2D,
                .format = Format::R8G8B8A8_UNORM,
                .extent = { 256, 256, 1 },
                .mipLevels = 1,
                .samples = SampleCountFlagBits::Samples1Bit,
                .usage = TextureUsageFlagBits::ColorAttachmentBit,
                .memoryUsage = MemoryUsage::GpuOnly,
        });
        const Texture depthResolveTexture = device.createTexture(TextureOptions{
                .type = TextureType::TextureType2D,
#if defined(KD_PLATFORM_MACOS)
                .format = Format::D32_SFLOAT_S8_UINT,
#else
                .format = Format::D24_UNORM_S8_UINT,
#endif
                .extent = { 256, 256, 1 },
                .mipLevels = 1,
                .samples = SampleCountFlagBits::Samples1Bit,
                .usage = TextureUsageFlagBits::DepthStencilAttachmentBit,
                .memoryUsage = MemoryUsage::GpuOnly,
        });

        const TextureView colorMSAATextureView = colorMSAATexture.createView(TextureViewOptions{
                .viewType = ViewType::ViewType2D,
                .range = {
                        .aspectMask = TextureAspectFlagBits::ColorBit,
                },
        });
        const TextureView depthMSAATextureView = depthMSAATexture.createView(TextureViewOptions{
                .viewType = ViewType::ViewType2D,
                .range = {
                        .aspectMask = (TextureAspectFlagBits::DepthBit | TextureAspectFlagBits::StencilBit),
                },
        });

        const TextureView colorResolveTextureView = colorResolveTexture.createView(TextureViewOptions{
                .viewType = ViewType::ViewType2D,
                .range = {
                        .aspectMask = TextureAspectFlagBits::ColorBit,
                },
        });
        const TextureView depthResolveTextureView = depthResolveTexture.createView(TextureViewOptions{
                .viewType = ViewType::ViewType2D,
                .range = {
                        .aspectMask = (TextureAspectFlagBits::DepthBit | TextureAspectFlagBits::StencilBit),
                },
        });

        const PipelineLayout pipelineLayout = device.createPipelineLayout();
        const GraphicsPipeline pipeline = device.createGraphicsPipeline(GraphicsPipelineOptions{
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
                        { .format = Format::R8G8B8A8_UNORM },
                },
                .depthStencil = {
                        .format = Format::D24_UNORM_S8_UINT,
                        .depthWritesEnabled = true,
                        .depthCompareOperation = CompareOperation::Less,
                        .resolveDepthStencil = true,
                },
                .multisample = {
                        .samples = samples,
                },
                .dynamicRendering = {
                        .enabled = true,
                },
        });

        // THEN
        REQUIRE(pipelineLayout.isValid());
        REQUIRE(pipeline.isValid());
        REQUIRE(colorMSAATextureView.isValid());
        REQUIRE(depthMSAATextureView.isValid());
        REQUIRE(colorResolveTextureView.isValid());
        REQUIRE(depthResolveTextureView.isValid());
        REQUIRE(device.isValid());

        SUBCASE("A constructed DynamicRenderPassCommandRecorder that Resolve MSAA Color & Depth with Vulkan API")
        {
            // GIVEN
            CommandRecorder commandRecorder = device.createCommandRecorder();
            const RenderPassCommandRecorderWithDynamicRenderingOptions renderPassOptions{
                .colorAttachments = {
                        { .view = colorMSAATextureView,
                          .resolveView = colorResolveTextureView,
                          .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f },
                          .finalLayout = TextureLayout::PresentSrc } },
                .depthStencilAttachment = {
                        .view = depthMSAATextureView,
                        .resolveView = depthResolveTextureView,
                        .depthResolveMode = ResolveModeFlagBits::Average,
                        .stencilResolveMode = ResolveModeFlagBits::None,
                },
                .samples = samples,
            };

            // WHEN
            DynamicRenderPassCommandRecorder renderPassRecorder = commandRecorder.beginRenderPass(renderPassOptions);
            renderPassRecorder.setPipeline(pipeline);
            renderPassRecorder.end();

            CommandBuffer commandBuffer = commandRecorder.finish();

            // THEN
            CHECK(commandRecorder.isValid());
            CHECK(commandBuffer.isValid());
            CHECK(renderPassRecorder.isValid());
        }
    }

    TEST_CASE("DynamicRenderPassCommandRecorder - Subpass Rendering" * doctest::skip(!hasDynamicRenderingSupport || !hasSubpassDynamicRenderingSupport))
    {
        const bool supportsAverageDepthResolveMode = discreteGPUAdapter->properties().depthResolveProperties.supportedDepthResolveModes.testFlag(ResolveModeFlagBits::Average);
        if (!supportsAverageDepthResolveMode)
            return;

        const SampleCountFlagBits samples = SampleCountFlagBits::Samples4Bit;
        const bool colorFrameBufferSupportsSamples = discreteGPUAdapter->properties().limits.framebufferColorSampleCounts.testFlag(samples);
        const bool depthFrameBufferSupportsSamples = discreteGPUAdapter->properties().limits.framebufferDepthSampleCounts.testFlag(samples);

        REQUIRE(discreteGPUAdapter->features().sampleRateShading);
        REQUIRE(supportsAverageDepthResolveMode);
        REQUIRE(colorFrameBufferSupportsSamples);
        REQUIRE(depthFrameBufferSupportsSamples);

        // GIVEN
        Device device = discreteGPUAdapter->createDevice(DeviceOptions{
                .requestedFeatures = {
                        .dynamicRendering = true,
                        .dynamicRenderingLocalRead = true,
                },
        });

        // THEN
        REQUIRE(device.isValid());

        const auto vertexShaderPath = assetPath() + "/shaders/tests/render_pass_command_recorder/triangle.vert.spv";
        auto vertexShader = device.createShaderModule(readShaderFile(vertexShaderPath));

        const auto fragmentShaderPath = assetPath() + "/shaders/tests/render_pass_command_recorder/triangle.frag.spv";
        auto fragmentShader = device.createShaderModule(readShaderFile(fragmentShaderPath));

        const Texture colorTexturePass1 = device.createTexture(TextureOptions{
                .type = TextureType::TextureType2D,
                .format = Format::R8G8B8A8_UNORM,
                .extent = { 256, 256, 1 },
                .mipLevels = 1,
                .usage = TextureUsageFlagBits::ColorAttachmentBit | TextureUsageFlagBits::InputAttachmentBit,
                .memoryUsage = MemoryUsage::GpuOnly,
        });

        const Texture colorTexturePass2 = device.createTexture(TextureOptions{
                .type = TextureType::TextureType2D,
                .format = Format::R8G8B8A8_UNORM,
                .extent = { 256, 256, 1 },
                .mipLevels = 1,
                .usage = TextureUsageFlagBits::ColorAttachmentBit,
                .memoryUsage = MemoryUsage::GpuOnly,
        });

        const TextureView colorTexturePass1View = colorTexturePass1.createView(TextureViewOptions{
                .viewType = ViewType::ViewType2D,
                .range = {
                        .aspectMask = TextureAspectFlagBits::ColorBit,
                },
        });

        const TextureView colorTexturePass2View = colorTexturePass2.createView(TextureViewOptions{
                .viewType = ViewType::ViewType2D,
                .range = {
                        .aspectMask = TextureAspectFlagBits::ColorBit,
                },
        });

        const PipelineLayout pipelineLayoutPass1 = device.createPipelineLayout();
        const GraphicsPipeline pipelinePass1 = device.createGraphicsPipeline(GraphicsPipelineOptions{
                .label = "pipelinePass1",
                .shaderStages = {
                        { .shaderModule = vertexShader.handle(), .stage = ShaderStageFlagBits::VertexBit },
                        { .shaderModule = fragmentShader.handle(), .stage = ShaderStageFlagBits::FragmentBit },
                },
                .layout = pipelineLayoutPass1.handle(),
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
                        // We need to specify all our RenderTarget even if we will only target 1
                        { .format = Format::R8G8B8A8_UNORM },
                        { .format = Format::R8G8B8A8_UNORM },
                },
                .depthStencil = {
                        .format = Format::D24_UNORM_S8_UINT,
                        .depthWritesEnabled = true,
                        .depthCompareOperation = CompareOperation::Less,
                        .resolveDepthStencil = true,
                },
                .dynamicRendering = {
                        .enabled = true,
                        .dynamicInputLocations = DynamicInputAttachmentLocations{
                                // Specify that we have no input attachments
                                .inputColorAttachments = {
                                        {
                                                .enabled = false,
                                        },
                                        {
                                                .enabled = false,
                                        },
                                },
                        },
                        .dynamicOutputLocations = DynamicOutputAttachmentLocations{
                                // Specify that we want frag output[0] to write only to color attachment[0]
                                .outputAttachments = {
                                        {
                                                .enabled = true,
                                                .remappedIndex = 0,
                                        },
                                        {
                                                .enabled = false,
                                        },
                                },
                        },
                },
        });

        const BindGroupLayout bindGroupLayoutPass2 = device.createBindGroupLayout(BindGroupLayoutOptions{
                .bindings = {
                        {
                                .binding = 0,
                                .resourceType = ResourceBindingType::InputAttachment,
                                .shaderStages = ShaderStageFlags(ShaderStageFlagBits::FragmentBit),
                        },
                },
        });

        const PipelineLayout pipelineLayoutPass2 = device.createPipelineLayout(PipelineLayoutOptions{
                .bindGroupLayouts = { bindGroupLayoutPass2 },
        });

        const GraphicsPipeline pipelinePass2 = device.createGraphicsPipeline(GraphicsPipelineOptions{
                .label = "pipelinePass2",
                .shaderStages = {
                        { .shaderModule = vertexShader.handle(), .stage = ShaderStageFlagBits::VertexBit },
                        { .shaderModule = fragmentShader.handle(), .stage = ShaderStageFlagBits::FragmentBit },
                },
                .layout = pipelineLayoutPass2.handle(),
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
                        // We need to specify all our RenderTarget even if we will only target 1
                        { .format = Format::R8G8B8A8_UNORM },
                        { .format = Format::R8G8B8A8_UNORM },
                },
                .depthStencil = {
                        .format = Format::D24_UNORM_S8_UINT,
                        .depthWritesEnabled = true,
                        .depthCompareOperation = CompareOperation::Less,
                        .resolveDepthStencil = true,
                },
                .dynamicRendering = {
                        .enabled = true,
                        .dynamicInputLocations = DynamicInputAttachmentLocations{
                                // Specify that we want color attachment[0] to be fed as input attachment[0]
                                .inputColorAttachments = {
                                        {
                                                .enabled = true,
                                                .remappedIndex = 0,
                                        },
                                        {
                                                .enabled = false,
                                        },
                                },
                        },
                        .dynamicOutputLocations = DynamicOutputAttachmentLocations{
                                // Specify that we want frag output[0] to write only to color attachment[1]
                                .outputAttachments = {
                                        {
                                                .enabled = false,
                                        },
                                        {
                                                .enabled = true,
                                                .remappedIndex = 0,
                                        },
                                },
                        },
                },
        });

        // THEN
        REQUIRE(pipelineLayoutPass1.isValid());
        REQUIRE(pipelinePass1.isValid());
        REQUIRE(bindGroupLayoutPass2.isValid());
        REQUIRE(pipelineLayoutPass2.isValid());
        REQUIRE(pipelinePass2.isValid());
        REQUIRE(colorTexturePass1.isValid());
        REQUIRE(colorTexturePass2.isValid());
        REQUIRE(colorTexturePass1View.isValid());
        REQUIRE(colorTexturePass2View.isValid());
        REQUIRE(device.isValid());

        // GIVEN
        CommandRecorder commandRecorder = device.createCommandRecorder();
        const RenderPassCommandRecorderWithDynamicRenderingOptions renderPassOptions{
            .colorAttachments = {
                    {
                            // Offscreen Texture (Pass 1)
                            .view = colorTexturePass1View, // We always render to the color texture
                            .clearValue = ColorClearValue{ 0.0f, 0.0f, 0.0f, 1.0f },
                            .layout = TextureLayout::DynamicLocalRead,
                    },
                    {
                            // Swapchain Output (Pass 2)
                            .view = colorTexturePass2View,
                            .clearValue = ColorClearValue{ 0.3f, 0.3f, 0.3f, 1.0f },
                            .layout = TextureLayout::ColorAttachmentOptimal,
                    },
            },
        };

        DynamicRenderPassCommandRecorder renderPassRecorder = commandRecorder.beginRenderPass(renderPassOptions);

        // WHEN
        std::array<std::optional<uint32_t>, 2> pass1InputIndices = { std::nullopt, std::nullopt }; // no input attachments
        std::array<std::optional<uint32_t>, 2> pass1OutputLocations = { 0, std::nullopt }; // frag output 0 -> attachment 0; attachment 1 unused
        renderPassRecorder.setInputAttachmentMapping(pass1InputIndices, {}, {});
        renderPassRecorder.setOutputAttachmentMapping(pass1OutputLocations);

        renderPassRecorder.setPipeline(pipelinePass1);

        // Technically we would need to sync input attachments writes with reads but since we don't draw anything...

        std::array<std::optional<uint32_t>, 2> pass2InputIndices = { 0, std::nullopt }; // input attachment 0 reads from attachment 0
        const std::array<std::optional<uint32_t>, 2> pass2OutputLocations = { std::nullopt, 0 }; // frag output 0 -> attachment 1; frag output 1 -> attachment 0 unused
        renderPassRecorder.setInputAttachmentMapping(pass2InputIndices, {}, {});
        renderPassRecorder.setOutputAttachmentMapping(pass2OutputLocations);

        renderPassRecorder.setPipeline(pipelinePass2);
        renderPassRecorder.end();

        CommandBuffer commandBuffer = commandRecorder.finish();

        // THEN
        CHECK(commandRecorder.isValid());
        CHECK(commandBuffer.isValid());
        CHECK(renderPassRecorder.isValid());
    }
}

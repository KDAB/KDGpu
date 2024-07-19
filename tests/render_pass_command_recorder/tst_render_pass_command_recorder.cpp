/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/graphics_pipeline.h>
#include <KDGpu/graphics_pipeline_options.h>
#include <KDGpu/render_pass_command_recorder.h>
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

#include <KDUtils/file.h>
#include <KDUtils/dir.h>

#include <type_traits>

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

TEST_SUITE("RenderPassCommandRecorder")
{
    // GIVEN
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "RenderPassCommandRecorder",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });

    Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);

    TEST_CASE("RenderPassCommandRecorder")
    {
        Device device = discreteGPUAdapter->createDevice();

        const auto vertexShaderPath = assetPath() + "/shaders/tests/render_pass_command_recorder/triangle.vert.spv";
        auto vertexShader = device.createShaderModule(readShaderFile(vertexShaderPath));

        const auto fragmentShaderPath = assetPath() + "/shaders/tests/render_pass_command_recorder/triangle.frag.spv";
        auto fragmentShader = device.createShaderModule(readShaderFile(fragmentShaderPath));

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
        });

        class RenderPass defaultRenderPass = device.createRenderPass(RenderPassOptions{
                .attachments = {
                        AttachmentDescription{
                                .format = Format::R8G8B8A8_UNORM,
                                .stencilLoadOperation = AttachmentLoadOperation::DontCare,
                                .stencilStoreOperation = AttachmentStoreOperation::DontCare,
                                .finalLayout = TextureLayout::ColorAttachmentOptimal,
                        },
                        AttachmentDescription{
                                .format = Format::D24_UNORM_S8_UINT,
                                .stencilLoadOperation = AttachmentLoadOperation::DontCare,
                                .stencilStoreOperation = AttachmentStoreOperation::DontCare,
                                .finalLayout = TextureLayout::DepthStencilAttachmentOptimal,
                        },
                },
                .subpassDescriptions = {
                        SubpassDescription{
                                .colorAttachmentReference = { { 0 } },
                                .depthAttachmentReference = { { 1 } },
                        },
                },
                .subpassDependencies = {
                        SubpassDependenciesDescriptions{
                                .srcSubpass = ExternalSubpass,
                                .dstSubpass = 0,
                                .srcStageMask = PipelineStageFlagBit::TopOfPipeBit,
                                .dstStageMask = PipelineStageFlagBit::AllGraphicsBit,
                                .srcAccessMask = AccessFlagBit::None,
                                .dstAccessMask = AccessFlagBit::ColorAttachmentWriteBit | AccessFlagBit::ColorAttachmentReadBit | AccessFlagBit::DepthStencilAttachmentWriteBit | AccessFlagBit::DepthStencilAttachmentReadBit | AccessFlagBit::InputAttachmentReadBit,
                        },
                        SubpassDependenciesDescriptions{
                                .srcSubpass = ExternalSubpass,
                                .dstSubpass = 0,
                                .srcStageMask = PipelineStageFlagBit::AllGraphicsBit,
                                .dstStageMask = PipelineStageFlagBit::BottomOfPipeBit,
                                .srcAccessMask = AccessFlagBit::ColorAttachmentWriteBit | AccessFlagBit::ColorAttachmentReadBit | AccessFlagBit::DepthStencilAttachmentWriteBit | AccessFlagBit::DepthStencilAttachmentReadBit | AccessFlagBit::InputAttachmentReadBit,
                                .dstAccessMask = AccessFlagBit::None,
                        },
                },
        });

        // THEN
        REQUIRE(pipelineLayout.isValid());
        REQUIRE(pipeline.isValid());
        REQUIRE(colorTextureView.isValid());
        REQUIRE(depthTextureView.isValid());
        REQUIRE(device.isValid());
        REQUIRE(defaultRenderPass.isValid());

        SUBCASE("Can't be default constructed")
        {
            // EXPECT
            REQUIRE(!std::is_default_constructible<RenderPassCommandRecorder>::value);
            REQUIRE(!std::is_trivially_default_constructible<RenderPassCommandRecorder>::value);
        }

        SUBCASE("RenderPassCommandRecorderOptions has sensible default values")
        {
            // GIVEN
            CommandRecorder commandRecorder = device.createCommandRecorder();
            const RenderPassCommandRecorderOptions renderPassOptions{
                .colorAttachments = {
                        { .view = colorTextureView,
                          .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f },
                          .finalLayout = TextureLayout::PresentSrc } },
                .depthStencilAttachment = {
                        .view = depthTextureView,
                }
            };

            // WHEN
            RenderPassCommandRecorder renderPassRecorder = commandRecorder.beginRenderPass(renderPassOptions);
            renderPassRecorder.setPipeline(pipeline);
            renderPassRecorder.end();

            CommandBuffer commandBuffer = commandRecorder.finish();

            // THEN
            CHECK(commandRecorder.isValid());
            CHECK(renderPassRecorder.isValid());
        }

        SUBCASE("RenderPassCommandRecorderWithRenderPassOptions has sensible default values")
        {
            // GIVEN
            CommandRecorder commandRecorder = device.createCommandRecorder();

            const RenderPassCommandRecorderWithRenderPassOptions renderPassOptions{
                .renderPass = defaultRenderPass,
                .attachments = {
                        {
                                .view = colorTextureView,
                                .color = Attachment::ColorOperations{},
                        },
                        {
                                .view = depthTextureView,
                                .depth = Attachment::DepthStencilOperations{},
                        },
                },
            };

            // WHEN
            RenderPassCommandRecorder renderPassRecorder = commandRecorder.beginRenderPass(renderPassOptions);
            renderPassRecorder.setPipeline(pipeline);
            renderPassRecorder.end();

            CommandBuffer commandBuffer = commandRecorder.finish();

            // THEN
            CHECK(commandRecorder.isValid());
            CHECK(renderPassRecorder.isValid());
            CHECK_EQ(renderPassOptions.attachments[1].depth->clearValue.depthClearValue, 1.0);
            CHECK_EQ(renderPassOptions.attachments[1].depth->clearValue.stencilClearValue, 0);
        }

        SUBCASE("A constructed RenderPassCommandRecorder from a Vulkan API")
        {
            // GIVEN
            CommandRecorder commandRecorder = device.createCommandRecorder();
            const RenderPassCommandRecorderOptions renderPassOptions{
                .colorAttachments = {
                        { .view = colorTextureView,
                          .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f },
                          .finalLayout = TextureLayout::PresentSrc } },
                .depthStencilAttachment = {
                        .view = depthTextureView,
                }
            };

            // WHEN
            RenderPassCommandRecorder renderPassRecorder = commandRecorder.beginRenderPass(renderPassOptions);
            renderPassRecorder.setPipeline(pipeline);
            renderPassRecorder.end();

            CommandBuffer commandBuffer = commandRecorder.finish();

            // THEN
            CHECK(commandRecorder.isValid());
            CHECK(renderPassRecorder.isValid());
        }

        SUBCASE("Uses different implicit RenderPasses if depth attachment is not used")
        {
            // GIVEN
            CommandRecorder commandRecorder = device.createCommandRecorder();
            const RenderPassCommandRecorderOptions renderPassOptionsWithDepth{
                .colorAttachments = {
                        { .view = colorTextureView,
                          .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f },
                          .finalLayout = TextureLayout::PresentSrc } },
                .depthStencilAttachment = {
                        .view = depthTextureView,
                }
            };
            const RenderPassCommandRecorderOptions renderPassOptionsWithoutDepth{
                .colorAttachments = {
                        { .view = colorTextureView,
                          .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f },
                          .finalLayout = TextureLayout::PresentSrc } },
            };

            const GraphicsPipeline depthPipeline = device.createGraphicsPipeline(GraphicsPipelineOptions{
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
            });

            const GraphicsPipeline noDepthPipeline = device.createGraphicsPipeline(GraphicsPipelineOptions{
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
            });

            // WHEN
            RenderPassCommandRecorder renderPassRecorderDepth = commandRecorder.beginRenderPass(renderPassOptionsWithDepth);
            renderPassRecorderDepth.setPipeline(depthPipeline);
            renderPassRecorderDepth.end();

            RenderPassCommandRecorder renderPassRecorderNoDepth = commandRecorder.beginRenderPass(renderPassOptionsWithoutDepth);
            renderPassRecorderNoDepth.setPipeline(noDepthPipeline);
            renderPassRecorderNoDepth.end();

            CommandBuffer commandBuffer = commandRecorder.finish();

            // THEN
            CHECK(commandRecorder.isValid());
            CHECK(renderPassRecorderDepth.isValid());
            CHECK(renderPassRecorderNoDepth.isValid());
        }

        SUBCASE("Destruction")
        {
            // GIVEN
            const RenderPassCommandRecorderOptions renderPassOptions{
                .colorAttachments = {
                        { .view = colorTextureView,
                          .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f },
                          .finalLayout = TextureLayout::PresentSrc } },
                .depthStencilAttachment = {
                        .view = depthTextureView,
                }
            };
            Handle<RenderPassCommandRecorder_t> recorderHandle;

            {
                // WHEN
                CommandRecorder commandRecorder = device.createCommandRecorder();
                RenderPassCommandRecorder renderPassRecorder = commandRecorder.beginRenderPass(renderPassOptions);
                recorderHandle = renderPassRecorder.handle();
                renderPassRecorder.end();

                CommandBuffer commandBuffer = commandRecorder.finish();

                // THEN
                CHECK(commandRecorder.isValid());
                CHECK(renderPassRecorder.isValid());
                CHECK(recorderHandle.isValid());
                CHECK(api->resourceManager()->getRenderPassCommandRecorder(recorderHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getRenderPassCommandRecorder(recorderHandle) == nullptr);
        }
    }

    TEST_CASE("RenderPassCommandRecorder - MultiView")
    {
        REQUIRE(discreteGPUAdapter->properties().multiViewProperties.maxMultiViewCount > 1);
        REQUIRE(discreteGPUAdapter->features().multiView);

        // GIVEN
        Device device = discreteGPUAdapter->createDevice(DeviceOptions{
                .requestedFeatures = {
                        .multiView = true,
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

        SUBCASE("A constructed RenderPassCommandRecorder with MultiView from a Vulkan API")
        {
            // GIVEN
            CommandRecorder commandRecorder = device.createCommandRecorder();
            const RenderPassCommandRecorderOptions renderPassOptions{
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
            RenderPassCommandRecorder renderPassRecorder = commandRecorder.beginRenderPass(renderPassOptions);
            renderPassRecorder.setPipeline(pipeline);
            renderPassRecorder.end();

            CommandBuffer commandBuffer = commandRecorder.finish();

            // THEN
            CHECK(commandRecorder.isValid());
            CHECK(renderPassRecorder.isValid());
        }
    }

    TEST_CASE("RenderPassCommandRecorder - Resolve MSAA Color & Depth")
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
                .viewType = ViewType::ViewType2DArray,
                .range = {
                        .aspectMask = TextureAspectFlagBits::ColorBit,
                },
        });
        const TextureView depthMSAATextureView = depthMSAATexture.createView(TextureViewOptions{
                .viewType = ViewType::ViewType2DArray,
                .range = {
                        .aspectMask = (TextureAspectFlagBits::DepthBit | TextureAspectFlagBits::StencilBit),
                },
        });

        const TextureView colorResolveTextureView = colorResolveTexture.createView(TextureViewOptions{
                .viewType = ViewType::ViewType2DArray,
                .range = {
                        .aspectMask = TextureAspectFlagBits::ColorBit,
                },
        });
        const TextureView depthResolveTextureView = depthResolveTexture.createView(TextureViewOptions{
                .viewType = ViewType::ViewType2DArray,
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
        });

        // THEN
        REQUIRE(pipelineLayout.isValid());
        REQUIRE(pipeline.isValid());
        REQUIRE(colorMSAATextureView.isValid());
        REQUIRE(depthMSAATextureView.isValid());
        REQUIRE(colorResolveTextureView.isValid());
        REQUIRE(depthResolveTextureView.isValid());
        REQUIRE(device.isValid());

        SUBCASE("A constructed RenderPassCommandRecorder that Resolve MSAA Color & Depth with Vulkan API")
        {
            // GIVEN
            CommandRecorder commandRecorder = device.createCommandRecorder();
            const RenderPassCommandRecorderOptions renderPassOptions{
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
            RenderPassCommandRecorder renderPassRecorder = commandRecorder.beginRenderPass(renderPassOptions);
            renderPassRecorder.setPipeline(pipeline);
            renderPassRecorder.end();

            CommandBuffer commandBuffer = commandRecorder.finish();

            // THEN
            CHECK(commandRecorder.isValid());
            CHECK(renderPassRecorder.isValid());
        }
    }

    constexpr bool hasSubpassSupport = true;
    TEST_CASE("RenderPassCommandRecorder - Subpass" * doctest::skip(!hasSubpassSupport))
    {
        Device device = discreteGPUAdapter->createDevice();

        const auto triangleVertexShaderPath = assetPath() + "/shaders/tests/render_pass_command_recorder/triangle.vert.spv";
        auto triangleVertexShader = device.createShaderModule(readShaderFile(triangleVertexShaderPath));

        const auto triangleFragmentShaderPath = assetPath() + "/shaders/tests/render_pass_command_recorder/triangle.frag.spv";
        auto triangleFragmentShader = device.createShaderModule(readShaderFile(triangleFragmentShaderPath));

        const auto readVertexShaderPath = assetPath() + "/shaders/tests/render_pass_command_recorder/read-image.vert.spv";
        auto readVertexShader = device.createShaderModule(readShaderFile(readVertexShaderPath));

        const auto readFragmentShaderPath = assetPath() + "/shaders/tests/render_pass_command_recorder/read-image.frag.spv";
        auto readFragmentShader = device.createShaderModule(readShaderFile(readFragmentShaderPath));

        Format depthFormat =
#if defined(KD_PLATFORM_MACOS)
                Format::D32_SFLOAT_S8_UINT;
#else
                Format::D24_UNORM_S8_UINT;
#endif

        RenderPass renderPass = device.createRenderPass(RenderPassOptions{
                .attachments = {
                        AttachmentDescription{
                                .stencilLoadOperation = AttachmentLoadOperation::DontCare,
                                .stencilStoreOperation = AttachmentStoreOperation::DontCare,
                        },
                        AttachmentDescription{
                                .format = depthFormat,
                                .loadOperation = AttachmentLoadOperation::DontCare,
                                .storeOperation = AttachmentStoreOperation::DontCare,
                                .stencilStoreOperation = AttachmentStoreOperation::DontCare,
                                .finalLayout = TextureLayout::DepthStencilAttachmentOptimal },
                        AttachmentDescription{
                                .stencilLoadOperation = AttachmentLoadOperation::DontCare,
                                .stencilStoreOperation = AttachmentStoreOperation::DontCare,
                                .finalLayout = TextureLayout::PresentSrc },

                },
                .subpassDescriptions = {
                        SubpassDescription{ .colorAttachmentReference = { { 0 } }, .depthAttachmentReference = { { 1 } } },
                        SubpassDescription{ .inputAttachmentReference = { { 0 } }, .colorAttachmentReference = { { 2 } } },
                },
                .subpassDependencies = {
                        SubpassDependenciesDescriptions{
                                .srcSubpass = 0,
                                .dstSubpass = 1,
                                .srcStageMask = PipelineStageFlagBit::ColorAttachmentOutputBit | PipelineStageFlagBit::EarlyFragmentTestBit,
                                .dstStageMask = PipelineStageFlagBit::FragmentShaderBit | PipelineStageFlagBit::ColorAttachmentOutputBit,
                                .srcAccessMask = AccessFlagBit::ColorAttachmentWriteBit | AccessFlagBit::DepthStencilAttachmentWriteBit,
                                .dstAccessMask = AccessFlagBit::InputAttachmentReadBit | AccessFlagBit::ColorAttachmentWriteBit,
                        },
                },
        });

        const Texture colorTexture = device.createTexture(TextureOptions{
                .type = TextureType::TextureType2D,
                .format = Format::R8G8B8A8_UNORM,
                .extent = { 256, 256, 1 },
                .mipLevels = 1,
                .samples = SampleCountFlagBits::Samples1Bit,
                .usage = TextureUsageFlagBits::ColorAttachmentBit | TextureUsageFlagBits::InputAttachmentBit,
                .memoryUsage = MemoryUsage::GpuOnly,
        });
        const Texture depthTexture = device.createTexture(TextureOptions{
                .type = TextureType::TextureType2D,
                .format = depthFormat,
                .extent = { 256, 256, 1 },
                .mipLevels = 1,
                .samples = SampleCountFlagBits::Samples1Bit,
                .usage = TextureUsageFlagBits::DepthStencilAttachmentBit,
                .memoryUsage = MemoryUsage::GpuOnly,
        });
        const Texture outputTexture = device.createTexture(TextureOptions{
                .type = TextureType::TextureType2D,
                .format = Format::R8G8B8A8_UNORM,
                .extent = { 256, 256, 1 },
                .mipLevels = 1,
                .samples = SampleCountFlagBits::Samples1Bit,
                .usage = TextureUsageFlagBits::ColorAttachmentBit,
                .memoryUsage = MemoryUsage::GpuOnly,
        });

        const TextureView colorTextureView = colorTexture.createView();
        const TextureView depthTextureView = depthTexture.createView();
        const TextureView outputTextureView = outputTexture.createView();

        const PipelineLayout pipelineLayoutSubpass0 = device.createPipelineLayout();

        const BindGroupLayoutOptions bindGroupLayoutOptions = {
            .bindings = { { .binding = 0,
                            .resourceType = ResourceBindingType::InputAttachment,
                            .shaderStages = ShaderStageFlags(ShaderStageFlagBits::FragmentBit) } }
        };

        auto colorBindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);

        const PipelineLayoutOptions pipelineLayoutOptions = {
            .bindGroupLayouts = { colorBindGroupLayout },
        };

        const PipelineLayout pipelineLayoutSubpass1 = device.createPipelineLayout(pipelineLayoutOptions);

        const GraphicsPipeline pipelineSubpass0 = device.createGraphicsPipeline(GraphicsPipelineOptions{
                .shaderStages = {
                        { .shaderModule = triangleVertexShader.handle(), .stage = ShaderStageFlagBits::VertexBit },
                        { .shaderModule = triangleFragmentShader.handle(), .stage = ShaderStageFlagBits::FragmentBit },
                },
                .layout = pipelineLayoutSubpass0.handle(),
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
                .depthStencil = { .format = depthFormat, .depthWritesEnabled = true, .depthCompareOperation = CompareOperation::Less },
                .renderPass = renderPass.handle(),
                .subpassIndex = 0,
        });

        const GraphicsPipeline pipelineSubpass1 = device.createGraphicsPipeline(GraphicsPipelineOptions{
                .shaderStages = {
                        { .shaderModule = readVertexShader.handle(), .stage = ShaderStageFlagBits::VertexBit },
                        { .shaderModule = readFragmentShader.handle(), .stage = ShaderStageFlagBits::FragmentBit },
                },
                .layout = pipelineLayoutSubpass1.handle(),
                .renderTargets = {
                        { .format = Format::R8G8B8A8_UNORM },
                },
                .renderPass = renderPass.handle(),
                .subpassIndex = 1,
        });

        // THEN
        REQUIRE(pipelineLayoutSubpass0.isValid());
        REQUIRE(pipelineLayoutSubpass1.isValid());
        REQUIRE(pipelineSubpass0.isValid());
        REQUIRE(pipelineSubpass1.isValid());
        REQUIRE(outputTextureView.isValid());
        REQUIRE(depthTextureView.isValid());
        REQUIRE(colorTextureView.isValid());
        REQUIRE(renderPass.isValid());
        REQUIRE(device.isValid());

        SUBCASE("Can move to next subpass")
        {
            // GIVEN
            CommandRecorder commandRecorder = device.createCommandRecorder();
            const RenderPassCommandRecorderWithRenderPassOptions renderPassOptions{
                .renderPass = renderPass.handle(),
                .attachments = {
                        {
                                .view = colorTextureView,
                                .color = Attachment::ColorOperations{
                                        .clearValue = ColorClearValue{ 0.3f, 0.3f, 0.3f, 1.0f },
                                },
                        },
                        {
                                .view = depthTextureView,
                                .depth = Attachment::DepthStencilOperations{},
                        },
                        {
                                .view = outputTextureView,
                                .color = Attachment::ColorOperations{
                                        .clearValue = ColorClearValue{ 0.3f, 0.3f, 0.3f, 1.0f },
                                },
                        },
                },
            };

            // WHEN
            RenderPassCommandRecorder renderPassRecorder = commandRecorder.beginRenderPass(renderPassOptions);
            renderPassRecorder.setPipeline(pipelineSubpass0);
            renderPassRecorder.nextSubpass();
            renderPassRecorder.setPipeline(pipelineSubpass1);
            renderPassRecorder.end();

            CommandBuffer commandBuffer = commandRecorder.finish();

            // THEN
            CHECK(commandRecorder.isValid());
            CHECK(renderPassRecorder.isValid());
            // And has no validation errors in console
        }
    }
}

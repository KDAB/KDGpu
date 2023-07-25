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
            .applicationVersion = SERENITY_MAKE_API_VERSION(0, 1, 0, 0) });

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
                .format = Format::D24_UNORM_S8_UINT,
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

        // THEN
        REQUIRE(pipelineLayout.isValid());
        REQUIRE(pipeline.isValid());
        REQUIRE(colorTextureView.isValid());
        REQUIRE(depthTextureView.isValid());
        REQUIRE(device.isValid());

        SUBCASE("Can't be default constructed")
        {
            // EXPECT
            REQUIRE(!std::is_default_constructible<RenderPassCommandRecorder>::value);
            REQUIRE(!std::is_trivially_default_constructible<RenderPassCommandRecorder>::value);
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
                .format = Format::D24_UNORM_S8_UINT,
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
}

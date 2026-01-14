/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/graphics_pipeline.h>
#include <KDGpu/graphics_pipeline_options.h>
#include <KDGpu/device.h>
#include <KDGpu/instance.h>
#include <KDGpu/render_pass.h>
#include <KDGpu/render_pass_options.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

#include <KDUtils/file.h>
#include <KDUtils/dir.h>

#include <array>

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

Format selectDepthFormat(Adapter *adapter)
{
    // Choose a depth format from the ones supported
    constexpr std::array<Format, 5> preferredDepthFormat = {
        Format::D24_UNORM_S8_UINT,
        Format::D16_UNORM_S8_UINT,
        Format::D32_SFLOAT_S8_UINT,
        Format::D16_UNORM,
        Format::D32_SFLOAT
    };

    for (const auto &depthFormat : preferredDepthFormat) {
        const FormatProperties formatProperties = adapter->formatProperties(depthFormat);
        if (formatProperties.optimalTilingFeatures & FormatFeatureFlagBit::DepthStencilAttachmentBit) {
            return depthFormat;
        }
    }

    return Format::UNDEFINED;
}

} // namespace

TEST_SUITE("GraphicsPipeline")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "GraphicsPipeline",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter *adapter = instance.selectAdapter(AdapterDeviceType::Default);
    Device device = adapter->createDevice(DeviceOptions{ .requestedFeatures = adapter->features() });

    const auto vertexShaderPath = assetPath() + "/shaders/tests/graphics_pipeline/triangle.vert.spv";
    auto vertexShader = device.createShaderModule(readShaderFile(vertexShaderPath));

    const auto fragmentShaderPath = assetPath() + "/shaders/tests/graphics_pipeline/triangle.frag.spv";
    auto fragmentShader = device.createShaderModule(readShaderFile(fragmentShaderPath));

    TEST_CASE("Construction")
    {
        REQUIRE(device.isValid());
        REQUIRE(vertexShader.isValid());
        REQUIRE(fragmentShader.isValid());

        SUBCASE("A default constructed GraphicsPipeline is invalid")
        {
            // GIVEN
            GraphicsPipeline c;
            // THEN
            REQUIRE(!c.isValid());
        }

        SUBCASE("A constructed GraphicsPipeline from a Vulkan API")
        {
            // GIVEN
            PipelineLayoutOptions pipelineLayoutOptions{};
            PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);
            const Format depthFormat = selectDepthFormat(adapter);
            REQUIRE(depthFormat != Format::UNDEFINED);

            // clang-format off
            GraphicsPipelineOptions pipelineOptions = {
                .shaderStages = {
                    { .shaderModule = vertexShader.handle(), .stage = ShaderStageFlagBits::VertexBit },
                    { .shaderModule = fragmentShader.handle(), .stage = ShaderStageFlagBits::FragmentBit }
                },
                .layout = pipelineLayout.handle(),
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
                    { .format = Format::R8G8B8A8_UNORM }
                },
                .depthStencil = {
                    .format = depthFormat,
                    .depthWritesEnabled = true,
                    .depthCompareOperation = CompareOperation::Less
                },
                .primitive = {
                    .lineWidth = adapter->features().wideLines ? 20.0f : 1.0f,
                }
            };
            // clang-format on

            // WHEN
            GraphicsPipeline g = device.createGraphicsPipeline(pipelineOptions);

            // THEN
            CHECK(g.isValid());
        }

        SUBCASE("Move constructor & move assignment")
        {
            // GIVEN
            PipelineLayoutOptions pipelineLayoutOptions{};
            PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);
            const Format depthFormat = selectDepthFormat(adapter);
            REQUIRE(depthFormat != Format::UNDEFINED);

            GraphicsPipelineOptions pipelineOptions = {
                .shaderStages = {
                        { .shaderModule = vertexShader.handle(), .stage = ShaderStageFlagBits::VertexBit },
                        { .shaderModule = fragmentShader.handle(), .stage = ShaderStageFlagBits::FragmentBit },
                },
                .layout = pipelineLayout.handle(),
                .vertex = {
                        .buffers = { { .binding = 0, .stride = 2 * 4 * sizeof(float) } },
                        .attributes = {
                                {
                                        .location = 0,
                                        .binding = 0,
                                        .format = Format::R32G32B32A32_SFLOAT,
                                }, // Position
                                {
                                        .location = 1,
                                        .binding = 0,
                                        .format = Format::R32G32B32A32_SFLOAT,
                                        .offset = 4 * sizeof(float),
                                }, // Color
                        },
                },
                .renderTargets = { { .format = Format::R8G8B8A8_UNORM } },
                .depthStencil = {
                        .format = depthFormat,
                        .depthWritesEnabled = true,
                        .depthCompareOperation = CompareOperation::Less,
                },
                .primitive = {
                        .lineWidth = adapter->features().wideLines ? 20.0f : 1.0f,
                }
            };

            // WHEN
            GraphicsPipeline g1 = device.createGraphicsPipeline(pipelineOptions);
            GraphicsPipeline g2 = std::move(g1);

            // THEN
            CHECK(!g1.isValid());
            CHECK(g2.isValid());

            // WHEN
            GraphicsPipeline g3 = device.createGraphicsPipeline(pipelineOptions);
            g3 = std::move(g2);

            // THEN
            CHECK(!g2.isValid());
            CHECK(g3.isValid());
        }

        SUBCASE("A GraphicsPipeline from a Vulkan API that does MSAA color and depth resolves")
        {
            // GIVEN
            PipelineLayoutOptions pipelineLayoutOptions{};
            PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);
            const Format depthFormat = selectDepthFormat(adapter);
            REQUIRE(depthFormat != Format::UNDEFINED);

            // clang-format off
            GraphicsPipelineOptions pipelineOptions = {
                .shaderStages = {
                    { .shaderModule = vertexShader.handle(), .stage = ShaderStageFlagBits::VertexBit },
                    { .shaderModule = fragmentShader.handle(), .stage = ShaderStageFlagBits::FragmentBit }
                },
                .layout = pipelineLayout.handle(),
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
                    { .format = Format::R8G8B8A8_UNORM }
                },
                .depthStencil = {
                    .format = depthFormat,
                    .depthWritesEnabled = true,
                    .depthCompareOperation = CompareOperation::Less,
                    .resolveDepthStencil = true,
                },
                .primitive = {
                    .lineWidth = adapter->features().wideLines ? 20.0f : 1.0f,
                },
                .multisample = {
                    .samples = SampleCountFlagBits::Samples4Bit,
                }
            };
            // clang-format on

            // WHEN
            GraphicsPipeline g = device.createGraphicsPipeline(pipelineOptions);

            // THEN
            CHECK(g.isValid());
        }
    }

    TEST_CASE("Destruction")
    {
        // GIVEN
        PipelineLayoutOptions pipelineLayoutOptions{};
        PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);
        const Format depthFormat = selectDepthFormat(adapter);
        REQUIRE(depthFormat != Format::UNDEFINED);

        // clang-format off
            GraphicsPipelineOptions pipelineOptions = {
                .shaderStages = {
                    { .shaderModule = vertexShader.handle(), .stage = ShaderStageFlagBits::VertexBit },
                    { .shaderModule = fragmentShader.handle(), .stage = ShaderStageFlagBits::FragmentBit }
                },
                .layout = pipelineLayout.handle(),
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
                    { .format = Format::R8G8B8A8_UNORM }
                },
                .depthStencil = {
                    .format = depthFormat,
                    .depthWritesEnabled = true,
                    .depthCompareOperation = CompareOperation::Less
                }
            };
        // clang-format on

        Handle<GraphicsPipeline_t> pipelineHandle;

        SUBCASE("Going Out Of Scope")
        {
            {
                // WHEN
                GraphicsPipeline g = device.createGraphicsPipeline(pipelineOptions);
                pipelineHandle = g.handle();

                // THEN
                CHECK(g.isValid());
                CHECK(pipelineHandle.isValid());
                CHECK(api->resourceManager()->getGraphicsPipeline(pipelineHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getGraphicsPipeline(pipelineHandle) == nullptr);
        }

        SUBCASE("Move assignment")
        {

            // WHEN
            GraphicsPipeline g = device.createGraphicsPipeline(pipelineOptions);
            pipelineHandle = g.handle();

            // THEN
            CHECK(g.isValid());
            CHECK(pipelineHandle.isValid());
            CHECK(api->resourceManager()->getGraphicsPipeline(pipelineHandle) != nullptr);

            // WHEN
            g = {};

            // THEN
            CHECK(api->resourceManager()->getGraphicsPipeline(pipelineHandle) == nullptr);
        }
    }

    TEST_CASE("Comparison")
    {
        SUBCASE("Compare default constructed ComputePipelines")
        {
            // GIVEN
            GraphicsPipeline a;
            GraphicsPipeline b;

            // THEN
            CHECK(a == b);
        }

        SUBCASE("Compare device created GraphicsPipeline")
        {
            // GIVEN
            PipelineLayoutOptions pipelineLayoutOptions{};
            PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);
            const Format depthFormat = selectDepthFormat(adapter);
            REQUIRE(depthFormat != Format::UNDEFINED);

            // clang-format off
            GraphicsPipelineOptions pipelineOptions = {
                .shaderStages = {
                    { .shaderModule = vertexShader.handle(), .stage = ShaderStageFlagBits::VertexBit },
                    { .shaderModule = fragmentShader.handle(), .stage = ShaderStageFlagBits::FragmentBit }
                },
                .layout = pipelineLayout.handle(),
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
                    { .format = Format::R8G8B8A8_UNORM }
                },
                .depthStencil = {
                    .format = depthFormat,
                    .depthWritesEnabled = true,
                    .depthCompareOperation = CompareOperation::Less
                }
            };
            // clang-format on

            // WHEN
            GraphicsPipeline a = device.createGraphicsPipeline(pipelineOptions);
            GraphicsPipeline b = device.createGraphicsPipeline(pipelineOptions);

            // THEN
            CHECK(a != b);
        }
    }

    TEST_CASE("Specialization Constants")
    {
        REQUIRE(device.isValid());

        SUBCASE("A GraphicsPipeline from a Vulkan API with specialization constants")
        {
            // GIVEN
            const auto sCVertexShaderPath = assetPath() + "/shaders/tests/graphics_pipeline/specialization_constants.vert.spv";
            auto sCVertexShader = device.createShaderModule(readShaderFile(sCVertexShaderPath));

            const auto sCFragmentShaderPath = assetPath() + "/shaders/tests/graphics_pipeline/specialization_constants.frag.spv";
            auto sCFragmentShader = device.createShaderModule(readShaderFile(sCFragmentShaderPath));

            PipelineLayoutOptions pipelineLayoutOptions{};
            PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);
            const Format depthFormat = selectDepthFormat(adapter);
            REQUIRE(depthFormat != Format::UNDEFINED);

            GraphicsPipelineOptions pipelineOptions = {
                .shaderStages = {
                        {
                                .shaderModule = sCVertexShader.handle(),
                                .stage = ShaderStageFlagBits::VertexBit,
                                .specializationConstants = {
                                        { .constantId = 0, .value = 16 },
                                        { .constantId = 1, .value = 32 },
                                },
                        },
                        {
                                .shaderModule = sCFragmentShader.handle(),
                                .stage = ShaderStageFlagBits::FragmentBit,
                                .specializationConstants = {
                                        { .constantId = 2, .value = 8 },
                                },

                        },
                },
                .layout = pipelineLayout.handle(),
                .vertex = {
                        .buffers = {
                                { .binding = 0, .stride = 2 * 4 * sizeof(float) },
                        },
                        .attributes = {
                                { .location = 0, .binding = 0, .format = Format::R32G32B32A32_SFLOAT }, // Position
                        },
                },
                .renderTargets = {
                        { .format = Format::R8G8B8A8_UNORM },
                },
                .depthStencil = {
                        .format = depthFormat,
                        .depthWritesEnabled = true,
                        .depthCompareOperation = CompareOperation::Less,
                        .resolveDepthStencil = true,
                },
                .primitive = {},
                .multisample = {}
            };

            // WHEN
            GraphicsPipeline g = device.createGraphicsPipeline(pipelineOptions);

            // THEN
            CHECK(g.isValid());
        }
    }

    TEST_CASE("Graphical Pipeline with RenderPass")
    {
        // GIVEN
        PipelineLayoutOptions pipelineLayoutOptions{};
        PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);
        const Format depthFormat = selectDepthFormat(adapter);
        REQUIRE(depthFormat != Format::UNDEFINED);

        RenderPassOptions renderPassOptions = {
            .attachments = { AttachmentDescription{
                                     .format = Format::R8G8B8A8_UNORM,
                                     .stencilLoadOperation = AttachmentLoadOperation::DontCare,
                                     .stencilStoreOperation = AttachmentStoreOperation::DontCare },
                             AttachmentDescription{
                                     .format = depthFormat,
                                     .loadOperation = AttachmentLoadOperation::DontCare,
                                     .storeOperation = AttachmentStoreOperation::DontCare,
                                     .finalLayout = TextureLayout::DepthStencilAttachmentOptimal } },
            .subpassDescriptions = { SubpassDescription{
                    .colorAttachmentReference = { { 0 } },
                    .depthAttachmentReference = { { 1 } } } },
            .subpassDependencies = { SubpassDependenciesDescriptions{
                    .srcSubpass = ExternalSubpass,
                    .dstSubpass = 0,
                    .srcStageMask = PipelineStageFlagBit::TopOfPipeBit,
                    .dstStageMask = PipelineStageFlagBit::ColorAttachmentOutputBit | PipelineStageFlagBit::EarlyFragmentTestBit,
                    .srcAccessMask = AccessFlagBit::None,
                    .dstAccessMask = AccessFlagBit::ColorAttachmentWriteBit | AccessFlagBit::DepthStencilAttachmentWriteBit,
                    .dependencyFlags = DependencyFlagBits::ByRegion } }
        };
        RenderPass renderPass = device.createRenderPass(renderPassOptions);
        REQUIRE(renderPass.isValid());

        GraphicsPipelineOptions pipelineOptions = {
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
            .depthStencil = { .format = depthFormat, .depthWritesEnabled = true, .depthCompareOperation = CompareOperation::Less },
            .primitive = {
                    .lineWidth = adapter->features().wideLines ? 20.0f : 1.0f,
            },
            .renderPass = renderPass.handle(),
            .subpassIndex = 0
        };

        SUBCASE("Construction")
        {
            // WHEN
            GraphicsPipeline g = device.createGraphicsPipeline(pipelineOptions);

            // THEN
            CHECK(g.isValid());
        }

        SUBCASE("Destruction")
        {
            Handle<GraphicsPipeline_t> pipelineHandle;
            {
                // WHEN
                GraphicsPipeline g = device.createGraphicsPipeline(pipelineOptions);
                pipelineHandle = g.handle();

                // THEN
                CHECK(g.isValid());
                CHECK(pipelineHandle.isValid());
                CHECK(api->resourceManager()->getGraphicsPipeline(pipelineHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getGraphicsPipeline(pipelineHandle) == nullptr);
            CHECK(api->resourceManager()->getRenderPass(renderPass.handle()) != nullptr);
        }
    }

    TEST_CASE("Hashing")
    {
        SUBCASE("Hashing GraphicsPipelineOptions")
        {
            // GIVEN
            PipelineLayoutOptions pipelineLayoutOptions{};
            PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);
            const Format depthFormat = selectDepthFormat(adapter);
            REQUIRE(depthFormat != Format::UNDEFINED);

            GraphicsPipelineOptions pipelineOptions = {
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
                                { .location = 1, .binding = 0, .format = Format::R32G32B32A32_SFLOAT, .offset = 4 * sizeof(float) } // Color
                        },
                },
                .renderTargets = {
                        { .format = Format::R8G8B8A8_UNORM },
                },
                .depthStencil = {
                        .format = depthFormat,
                        .depthWritesEnabled = true,
                        .depthCompareOperation = CompareOperation::Less,
                },
            };

            // WHEN
            size_t hashValue1 = std::hash<GraphicsPipelineOptions>()(pipelineOptions);
            size_t hashValue2 = std::hash<GraphicsPipelineOptions>()(pipelineOptions);

            // THEN
            CHECK(hashValue1 == hashValue2);
        }
    }
}

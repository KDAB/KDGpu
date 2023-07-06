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
#include <KDGpu/vulkan/vulkan_graphics_api.h>

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
            .applicationVersion = SERENITY_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter *adapter = instance.selectAdapter(AdapterDeviceType::Default);
    Device device = adapter->createDevice();

    const auto vertexShaderPath = assetPath() + "/shaders/tests/graphics_pipeline/triangle.vert.spv";
    auto vertexShader = device.createShaderModule(KDGpu::readShaderFile(vertexShaderPath));

    const auto fragmentShaderPath = assetPath() + "/shaders/tests/graphics_pipeline/triangle.frag.spv";
    auto fragmentShader = device.createShaderModule(KDGpu::readShaderFile(fragmentShaderPath));

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
}

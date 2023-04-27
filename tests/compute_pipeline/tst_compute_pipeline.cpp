/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/compute_pipeline.h>
#include <KDGpu/compute_pipeline_options.h>
#include <KDGpu/device.h>
#include <KDGpu/instance.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

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
} // namespace

TEST_SUITE("ComputePipeline")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "ComputePipeline",
            .applicationVersion = SERENITY_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);
    Device device = discreteGPUAdapter->createDevice();

    const auto computeShaderPath = assetPath() + "/shaders/tests/compute_pipeline/empty_compute.comp.spv";
    auto computeShader = device.createShaderModule(KDGpu::readShaderFile(computeShaderPath));

    TEST_CASE("Construction")
    {
        SUBCASE("A default constructed ComputePipeline is invalid")
        {
            // GIVEN
            ComputePipeline c;
            // THEN
            REQUIRE(!c.isValid());
        }

        SUBCASE("A constructed ComputePipeline from a Vulkan API")
        {
            // GIVEN
            PipelineLayoutOptions pipelineLayoutOptions{};
            PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);

            // clang-format off
            const ComputePipelineOptions computePipelineOptions {
                .layout = pipelineLayout,
                .shaderStage = ComputeShaderStage {
                    .shaderModule = computeShader.handle()
                }
            };
            // clang-format on

            // WHEN
            ComputePipeline c = device.createComputePipeline(computePipelineOptions);

            // THEN
            CHECK(c.isValid());
        }
    }

    TEST_CASE("Destruction")
    {
        // GIVEN
        PipelineLayoutOptions pipelineLayoutOptions{};
        PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);

        // clang-format off
        const ComputePipelineOptions computePipelineOptions {
            .layout = pipelineLayout,
            .shaderStage = ComputeShaderStage {
                .shaderModule = computeShader.handle()
            }
        };
        // clang-format on

        Handle<ComputePipeline_t> pipelineHandle;

        SUBCASE("Going Out Of Scope")
        {
            {
                // WHEN
                ComputePipeline c = device.createComputePipeline(computePipelineOptions);
                pipelineHandle = c.handle();

                // THEN
                CHECK(c.isValid());
                CHECK(pipelineHandle.isValid());
                CHECK(api->resourceManager()->getComputePipeline(pipelineHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getComputePipeline(pipelineHandle) == nullptr);
        }

        SUBCASE("Move assignment")
        {
            // WHEN
            ComputePipeline c = device.createComputePipeline(computePipelineOptions);
            pipelineHandle = c.handle();

            // THEN
            CHECK(c.isValid());
            CHECK(pipelineHandle.isValid());
            CHECK(api->resourceManager()->getComputePipeline(pipelineHandle) != nullptr);

            // WHEN
            c = {};

            // THEN
            CHECK(api->resourceManager()->getComputePipeline(pipelineHandle) == nullptr);
        }
    }

    TEST_CASE("Comparison")
    {
        SUBCASE("Compare default constructed ComputePipelines")
        {
            // GIVEN
            ComputePipeline a;
            ComputePipeline b;

            // THEN
            CHECK(a == b);
        }

        SUBCASE("Compare device created ComputePipelines")
        {
            // GIVEN
            PipelineLayoutOptions pipelineLayoutOptions{};
            PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);

            // clang-format off
            const ComputePipelineOptions computePipelineOptions {
                .layout = pipelineLayout,
                .shaderStage = ComputeShaderStage {
                    .shaderModule = computeShader.handle()
                }
            };
            // clang-format on

            // WHEN
            ComputePipeline a = device.createComputePipeline(computePipelineOptions);
            ComputePipeline b = device.createComputePipeline(computePipelineOptions);

            // THEN
            CHECK(a != b);
        }
    }
}

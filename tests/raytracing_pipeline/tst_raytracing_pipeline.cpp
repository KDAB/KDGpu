/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/raytracing_pipeline.h>
#include <KDGpu/raytracing_pipeline_options.h>
#include <KDGpu/device.h>
#include <KDGpu/instance.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

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

TEST_SUITE("RayTracingPipeline")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "RayTracingPipeline",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);
    Device device = discreteGPUAdapter->createDevice(DeviceOptions{
            .requestedFeatures = discreteGPUAdapter->features(),
    });
    const bool supportsRayTracing = discreteGPUAdapter->features().rayTracingPipeline;

    const auto rayTracingGenShaderPath = assetPath() + "/shaders/tests/raytracing_pipeline/raygen.spv";
    const auto rayTracingGenShaderSCPath = assetPath() + "/shaders/tests/raytracing_pipeline/raygensc.spv";
    const auto rayTracingMissShaderPath = assetPath() + "/shaders/tests/raytracing_pipeline/miss.spv";
    const auto rayTracingClosestShaderPath = assetPath() + "/shaders/tests/raytracing_pipeline/closest.spv";

    const BindGroupLayoutOptions bindGroupLayoutOptions = {
        .bindings = {
                {
                        // Acceleration Structure
                        .binding = 0,
                        .count = 1,
                        .resourceType = ResourceBindingType::AccelerationStructure,
                        .shaderStages = ShaderStageFlags(ShaderStageFlagBits::RaygenBit),
                },
                {
                        // Output Image
                        .binding = 1,
                        .count = 1,
                        .resourceType = ResourceBindingType::StorageImage,
                        .shaderStages = ShaderStageFlagBits::RaygenBit | ShaderStageFlagBits::MissBit | ShaderStageFlagBits::ClosestHitBit,
                },
        },
    };

    TEST_CASE("Construction" * doctest::skip(!supportsRayTracing))
    {
        auto rayTracingGenShader = device.createShaderModule(readShaderFile(rayTracingGenShaderPath));
        auto rayTracingMissShader = device.createShaderModule(readShaderFile(rayTracingMissShaderPath));
        auto rayTracingClosestShader = device.createShaderModule(readShaderFile(rayTracingClosestShaderPath));

        const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);

        SUBCASE("A default constructed RayTracingPipeline is invalid")
        {
            // GIVEN
            RayTracingPipeline c;
            // THEN
            REQUIRE(!c.isValid());
        }

        SUBCASE("A constructed RayTracingPipeline from a Vulkan API")
        {
            // GIVEN
            PipelineLayoutOptions pipelineLayoutOptions{
                .bindGroupLayouts = { bindGroupLayout },
            };
            PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);

            const RayTracingPipelineOptions rayTracingPipelineOptions{
                .shaderStages = {
                        ShaderStage{
                                .shaderModule = rayTracingGenShader.handle(),
                                .stage = ShaderStageFlagBits::RaygenBit,
                        },
                        ShaderStage{
                                .shaderModule = rayTracingMissShader.handle(),
                                .stage = ShaderStageFlagBits::MissBit,
                        },
                        ShaderStage{
                                .shaderModule = rayTracingClosestShader.handle(),
                                .stage = ShaderStageFlagBits::ClosestHitBit,
                        },
                },
                .shaderGroups = {
                        // Gen
                        RayTracingShaderGroupOptions{
                                .type = RayTracingShaderGroupType::General,
                                .generalShaderIndex = 0,
                        },
                        // Miss
                        RayTracingShaderGroupOptions{
                                .type = RayTracingShaderGroupType::General,
                                .generalShaderIndex = 1,
                        },
                        // Closest Hit
                        RayTracingShaderGroupOptions{
                                .type = RayTracingShaderGroupType::TrianglesHit,
                                .generalShaderIndex = 0,
                                .closestHitShaderIndex = 2,
                        },
                },
                .layout = pipelineLayout,
            };

            // WHEN
            RayTracingPipeline c = device.createRayTracingPipeline(rayTracingPipelineOptions);

            // THEN
            CHECK(c.isValid());
        }
    }

    TEST_CASE("Destruction" * doctest::skip(!supportsRayTracing))
    {
        // GIVEN
        auto rayTracingGenShader = device.createShaderModule(readShaderFile(rayTracingGenShaderPath));
        auto rayTracingMissShader = device.createShaderModule(readShaderFile(rayTracingMissShaderPath));
        auto rayTracingClosestShader = device.createShaderModule(readShaderFile(rayTracingClosestShaderPath));

        const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);

        PipelineLayoutOptions pipelineLayoutOptions{
            .bindGroupLayouts = { bindGroupLayout },
        };
        PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);

        const RayTracingPipelineOptions rayTracingPipelineOptions{
            .shaderStages = {
                    ShaderStage{
                            .shaderModule = rayTracingGenShader.handle(),
                            .stage = ShaderStageFlagBits::RaygenBit,
                    },
                    ShaderStage{
                            .shaderModule = rayTracingMissShader.handle(),
                            .stage = ShaderStageFlagBits::MissBit,
                    },
                    ShaderStage{
                            .shaderModule = rayTracingClosestShader.handle(),
                            .stage = ShaderStageFlagBits::ClosestHitBit,
                    },
            },
            .shaderGroups = {
                    // Gen
                    RayTracingShaderGroupOptions{
                            .type = RayTracingShaderGroupType::General,
                            .generalShaderIndex = 0,
                    },
                    // Miss
                    RayTracingShaderGroupOptions{
                            .type = RayTracingShaderGroupType::General,
                            .generalShaderIndex = 1,
                    },
                    // Closest Hit
                    RayTracingShaderGroupOptions{
                            .type = RayTracingShaderGroupType::TrianglesHit,
                            .generalShaderIndex = 0,
                            .closestHitShaderIndex = 2,
                    },
            },
            .layout = pipelineLayout,
        };

        Handle<RayTracingPipeline_t> pipelineHandle;

        SUBCASE("Going Out Of Scope")
        {
            {
                // WHEN
                RayTracingPipeline c = device.createRayTracingPipeline(rayTracingPipelineOptions);
                pipelineHandle = c.handle();

                // THEN
                CHECK(c.isValid());
                CHECK(pipelineHandle.isValid());
                CHECK(api->resourceManager()->getRayTracingPipeline(pipelineHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getRayTracingPipeline(pipelineHandle) == nullptr);
        }

        SUBCASE("Move assignment")
        {
            // WHEN
            RayTracingPipeline c = device.createRayTracingPipeline(rayTracingPipelineOptions);
            pipelineHandle = c.handle();

            // THEN
            CHECK(c.isValid());
            CHECK(pipelineHandle.isValid());
            CHECK(api->resourceManager()->getRayTracingPipeline(pipelineHandle) != nullptr);

            // WHEN
            c = {};

            // THEN
            CHECK(api->resourceManager()->getRayTracingPipeline(pipelineHandle) == nullptr);
        }
    }

    TEST_CASE("Comparison" * doctest::skip(!supportsRayTracing))
    {
        auto rayTracingGenShader = device.createShaderModule(readShaderFile(rayTracingGenShaderPath));
        auto rayTracingMissShader = device.createShaderModule(readShaderFile(rayTracingMissShaderPath));
        auto rayTracingClosestShader = device.createShaderModule(readShaderFile(rayTracingClosestShaderPath));

        const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);

        SUBCASE("Compare default constructed RayTracingPipelines")
        {
            // GIVEN
            RayTracingPipeline a;
            RayTracingPipeline b;

            // THEN
            CHECK(a == b);
        }

        SUBCASE("Compare device created RayTracingPipelines")
        {
            // GIVEN
            PipelineLayoutOptions pipelineLayoutOptions{
                .bindGroupLayouts = { bindGroupLayout },
            };
            PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);

            const RayTracingPipelineOptions rayTracingPipelineOptions{
                .shaderStages = {
                        ShaderStage{
                                .shaderModule = rayTracingGenShader.handle(),
                                .stage = ShaderStageFlagBits::RaygenBit,
                        },
                        ShaderStage{
                                .shaderModule = rayTracingMissShader.handle(),
                                .stage = ShaderStageFlagBits::MissBit,
                        },
                        ShaderStage{
                                .shaderModule = rayTracingClosestShader.handle(),
                                .stage = ShaderStageFlagBits::ClosestHitBit,
                        },
                },
                .shaderGroups = {
                        // Gen
                        RayTracingShaderGroupOptions{
                                .type = RayTracingShaderGroupType::General,
                                .generalShaderIndex = 0,
                        },
                        // Miss
                        RayTracingShaderGroupOptions{
                                .type = RayTracingShaderGroupType::General,
                                .generalShaderIndex = 1,
                        },
                        // Closest Hit
                        RayTracingShaderGroupOptions{
                                .type = RayTracingShaderGroupType::TrianglesHit,
                                .generalShaderIndex = 0,
                                .closestHitShaderIndex = 2,
                        },
                },
                .layout = pipelineLayout,
            };

            // WHEN
            RayTracingPipeline a = device.createRayTracingPipeline(rayTracingPipelineOptions);
            RayTracingPipeline b = device.createRayTracingPipeline(rayTracingPipelineOptions);

            // THEN
            CHECK(a != b);
        }
    }

    TEST_CASE("Specialization Constants" * doctest::skip(!supportsRayTracing))
    {
        auto rayTracingGenShaderSC = device.createShaderModule(readShaderFile(rayTracingGenShaderSCPath));
        auto rayTracingMissShader = device.createShaderModule(readShaderFile(rayTracingMissShaderPath));
        auto rayTracingClosestShader = device.createShaderModule(readShaderFile(rayTracingClosestShaderPath));

        const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);

        SUBCASE("A constructed RayTracingPipeline from a Vulkan API with specialization constants")
        {
            // GIVEN

            PipelineLayoutOptions pipelineLayoutOptions{
                .bindGroupLayouts = { bindGroupLayout },
            };
            PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);

            const RayTracingPipelineOptions rayTracingPipelineOptions{
                .shaderStages = {
                        ShaderStage{
                                .shaderModule = rayTracingGenShaderSC.handle(),
                                .stage = ShaderStageFlagBits::RaygenBit,
                                .specializationConstants = {
                                        { .constantId = 0, .value = 0.001f },
                                        { .constantId = 1, .value = 10000.0f },
                                },
                        },
                        ShaderStage{
                                .shaderModule = rayTracingMissShader.handle(),
                                .stage = ShaderStageFlagBits::MissBit,
                        },
                        ShaderStage{
                                .shaderModule = rayTracingClosestShader.handle(),
                                .stage = ShaderStageFlagBits::ClosestHitBit,
                        },
                },
                .shaderGroups = {
                        // Gen
                        RayTracingShaderGroupOptions{
                                .type = RayTracingShaderGroupType::General,
                                .generalShaderIndex = 0,
                        },
                        // Miss
                        RayTracingShaderGroupOptions{
                                .type = RayTracingShaderGroupType::General,
                                .generalShaderIndex = 1,
                        },
                        // Closest Hit
                        RayTracingShaderGroupOptions{
                                .type = RayTracingShaderGroupType::TrianglesHit,
                                .generalShaderIndex = 0,
                                .closestHitShaderIndex = 2,
                        },
                },
                .layout = pipelineLayout,
            };

            // WHEN
            RayTracingPipeline c = device.createRayTracingPipeline(rayTracingPipelineOptions);

            // THEN
            CHECK(c.isValid());
        }
    }
}

/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/bind_group.h>
#include <KDGpu/bind_group_layout.h>
#include <KDGpu/bind_group_layout_options.h>
#include <KDGpu/bind_group_options.h>
#include <KDGpu/buffer.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/command_recorder.h>
#include <KDGpu/device.h>
#include <KDGpu/device_options.h>
#include <KDGpu/instance.h>
#include <KDGpu/pipeline_layout_options.h>
#include <KDGpu/queue.h>
#include <KDGpu/texture.h>
#include <KDGpu/texture_options.h>
#include <KDGpu/raytracing_pass_command_recorder.h>
#include <KDGpu/raytracing_pipeline.h>
#include <KDGpu/raytracing_pipeline_options.h>
#include <KDGpu/raytracing_shader_binding_table.h>
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

TEST_SUITE("RayTracingPassCommandRecorder")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "RayTracingPassCommandRecorder",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);
    Device device = discreteGPUAdapter->createDevice(DeviceOptions{
            .requestedFeatures = discreteGPUAdapter->features(),
    });
    const bool supportsRayTracing = discreteGPUAdapter->features().rayTracingPipeline;

    const auto rayTracingGenShaderPath = assetPath() + "/shaders/tests/raytracing_pipeline/raygen.spv";
    const auto rayTracingMissShaderPath = assetPath() + "/shaders/tests/raytracing_pipeline/miss.spv";
    const auto rayTracingClosestShaderPath = assetPath() + "/shaders/tests/raytracing_pipeline/closest.spv";

    TEST_CASE("Construction/Destruction" * doctest::skip(!supportsRayTracing))
    {
        SUBCASE("Can't be default constructed")
        {
            // EXPECT
            REQUIRE(!std::is_default_constructible<RayTracingPassCommandRecorder>::value);
            REQUIRE(!std::is_trivially_default_constructible<RayTracingPassCommandRecorder>::value);
        }

        SUBCASE("A constructed RayTracingPassCommandRecorder from a Vulkan API")
        {
            // GIVEN
            CommandRecorder commandRecorder = device.createCommandRecorder();

            // THEN
            CHECK(commandRecorder.isValid());

            // WHEN
            RayTracingPassCommandRecorder rayTracingCommandRecorder = commandRecorder.beginRayTracingPass();

            // THEN
            CHECK(rayTracingCommandRecorder.isValid());
        }

        SUBCASE("Move constructor & move assignment")
        {
            // GIVEN
            CommandRecorder commandRecorder = device.createCommandRecorder();

            // WHEN
            RayTracingPassCommandRecorder recorder1 = commandRecorder.beginRayTracingPass();
            RayTracingPassCommandRecorder recorder2(std::move(recorder1));

            // THEN
            CHECK(!recorder1.isValid());
            CHECK(recorder2.isValid());

            // WHEN
            RayTracingPassCommandRecorder recorder3 = commandRecorder.beginRayTracingPass();
            recorder3.end();
            const auto recorder2Handle = recorder2.handle();
            recorder3 = std::move(recorder2);

            // THEN
            CHECK(!recorder2.isValid());
            CHECK(recorder3.isValid());
            CHECK(recorder3.handle() == recorder2Handle);
        }

        SUBCASE("Destruction")
        {
            CommandRecorder commandRecorder = device.createCommandRecorder();
            Handle<RayTracingPassCommandRecorder_t> recorderHandle;

            {
                // WHEN
                RayTracingPassCommandRecorder rayTracingCommandRecorder = commandRecorder.beginRayTracingPass();
                recorderHandle = rayTracingCommandRecorder.handle();

                // THEN
                CHECK(commandRecorder.isValid());
                CHECK(rayTracingCommandRecorder.isValid());
                CHECK(recorderHandle.isValid());
                CHECK(api->resourceManager()->getRayTracingPassCommandRecorder(recorderHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getRayTracingPassCommandRecorder(recorderHandle) == nullptr);
        }
    }

    TEST_CASE("RayTracingPassCommandRecorder" * doctest::skip(!supportsRayTracing))
    {
        // GIVEN
        auto rayTracingGenShader = device.createShaderModule(readShaderFile(rayTracingGenShaderPath));
        auto rayTracingMissShader = device.createShaderModule(readShaderFile(rayTracingMissShaderPath));
        auto rayTracingClosestShader = device.createShaderModule(readShaderFile(rayTracingClosestShaderPath));

        const BindGroupLayoutOptions bindGroupLayoutOptions = {
            .bindings = {
                    {
                            .binding = 0,
                            .count = 1,
                            .resourceType = ResourceBindingType::AccelerationStructure,
                            .shaderStages = ShaderStageFlagBits::RaygenBit,
                    },
                    {
                            .binding = 1,
                            .count = 1,
                            .resourceType = ResourceBindingType::StorageImage,
                            .shaderStages = ShaderStageFlagBits::RaygenBit,
                    },
            },
        };
        const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);

        const PipelineLayoutOptions pipelineLayoutOptions{
            .bindGroupLayouts = { bindGroupLayout },
            .pushConstantRanges = {
                    PushConstantRange{
                            .offset = 0,
                            .size = sizeof(uint32_t),
                            .shaderStages = ShaderStageFlags(ShaderStageFlagBits::RaygenBit),
                    },
            },
        };
        const PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);

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
                    RayTracingShaderGroupOptions{
                            .type = RayTracingShaderGroupType::General,
                            .generalShaderIndex = 0,
                    },
                    RayTracingShaderGroupOptions{
                            .type = RayTracingShaderGroupType::General,
                            .generalShaderIndex = 1,
                    },
                    RayTracingShaderGroupOptions{
                            .type = RayTracingShaderGroupType::TrianglesHit,
                            .closestHitShaderIndex = 2,
                    },
            },
            .layout = pipelineLayout,
        };
        const RayTracingPipeline pipeline = device.createRayTracingPipeline(rayTracingPipelineOptions);

        const Texture storageImage = device.createTexture(TextureOptions{
                .type = TextureType::TextureType2D,
                .format = Format::R8G8B8A8_UNORM,
                .extent = { 256, 256, 1 },
                .mipLevels = 1,
                .samples = SampleCountFlagBits::Samples1Bit,
                .usage = TextureUsageFlagBits::StorageBit | TextureUsageFlagBits::TransferDstBit,
                .memoryUsage = MemoryUsage::GpuOnly,
        });

        const TextureView storageImageView = storageImage.createView(TextureViewOptions{
                .viewType = ViewType::ViewType2D,
                .range = {
                        .aspectMask = TextureAspectFlagBits::ColorBit,
                },
        });

        BindGroup bindGroup = device.createBindGroup(BindGroupOptions{
                .layout = bindGroupLayout,
                .resources = {
                        {
                                .binding = 1,
                                .resource = ImageBinding{ .textureView = storageImageView },
                        },
                },
        });

        // THEN
        REQUIRE(rayTracingGenShader.isValid());
        REQUIRE(rayTracingMissShader.isValid());
        REQUIRE(rayTracingClosestShader.isValid());
        REQUIRE(bindGroupLayout.isValid());
        REQUIRE(pipelineLayout.isValid());
        REQUIRE(pipeline.isValid());
        REQUIRE(bindGroup.isValid());

        SUBCASE("setPipeline")
        {
            // GIVEN
            CommandRecorder commandRecorder = device.createCommandRecorder();

            // WHEN
            RayTracingPassCommandRecorder recorder = commandRecorder.beginRayTracingPass();
            recorder.setPipeline(pipeline);
            recorder.end();
            CommandBuffer commandBuffer = commandRecorder.finish();

            // THEN
            CHECK(commandRecorder.isValid());
            CHECK(recorder.isValid());
            CHECK(commandBuffer.isValid());
        }

        SUBCASE("setBindGroup")
        {
            // GIVEN
            CommandRecorder commandRecorder = device.createCommandRecorder();

            // WHEN
            RayTracingPassCommandRecorder recorder = commandRecorder.beginRayTracingPass();
            recorder.setPipeline(pipeline);
            recorder.setBindGroup(0, bindGroup);
            recorder.end();
            CommandBuffer commandBuffer = commandRecorder.finish();

            // THEN
            CHECK(commandRecorder.isValid());
            CHECK(recorder.isValid());
            CHECK(commandBuffer.isValid());
        }

        SUBCASE("pushConstant")
        {
            // GIVEN
            CommandRecorder commandRecorder = device.createCommandRecorder();

            const uint32_t pushValue = 42;
            const PushConstantRange pushConstantRange{
                .offset = 0,
                .size = sizeof(uint32_t),
                .shaderStages = ShaderStageFlags(ShaderStageFlagBits::RaygenBit),
            };

            // WHEN
            RayTracingPassCommandRecorder recorder = commandRecorder.beginRayTracingPass();
            recorder.setPipeline(pipeline);
            recorder.pushConstant(pushConstantRange, &pushValue);
            recorder.end();
            CommandBuffer commandBuffer = commandRecorder.finish();

            // THEN
            CHECK(commandRecorder.isValid());
            CHECK(recorder.isValid());
            CHECK(commandBuffer.isValid());
        }

        SUBCASE("traceRays")
        {
            // GIVEN
            RayTracingShaderBindingTable sbt(&device, RayTracingShaderBindingTableOptions{
                                                              .nbrMissShaders = 1,
                                                              .nbrHitShaders = 1,
                                                      });
            sbt.addRayGenShaderGroup(pipeline, 0);
            sbt.addMissShaderGroup(pipeline, 1, 0);
            sbt.addHitShaderGroup(pipeline, 2, 0);

            REQUIRE(sbt.buffer().isValid());

            // WHEN -> creating Acceleration Structures
            struct Vertex {
                float x, y, z;
            };

            KDGpu::Buffer vertexBuffer = device.createBuffer(BufferOptions{
                    .size = 6 * sizeof(Vertex),
                    .usage = BufferUsageFlagBits::TransferSrcBit | BufferUsageFlagBits::TransferDstBit | BufferUsageFlagBits::AccelerationStructureBuildInputReadOnlyBit | BufferUsageFlagBits::ShaderDeviceAddressBit,
                    .memoryUsage = MemoryUsage::CpuToGpu,
            });

            const std::vector<Vertex> vertices{
                { 0.0f, 1.0f, 0.0f },
                { -1.0f, -1.0f, 0.0f },
                { 1.0f, -1.0f, 0.0f },
            };
            std::memcpy(vertexBuffer.map(), vertices.data(), vertices.size() * sizeof(Vertex));

            const AccelerationStructure bottomLevelAs = device.createAccelerationStructure(AccelerationStructureOptions{
                    .label = "BottomLevelAS",
                    .type = AccelerationStructureType::BottomLevel,
                    .flags = AccelerationStructureFlagBits::PreferFastTrace,
                    .geometryTypesAndCount = {
                            {
                                    .geometry = AccelerationStructureGeometryTrianglesData{
                                            .vertexFormat = Format::R32G32B32_SFLOAT,
                                            .vertexData = vertexBuffer,
                                            .vertexStride = sizeof(Vertex),
                                            .maxVertex = 2, // This is an index not a count
                                    },
                                    .maxPrimitiveCount = 1,
                            },
                    },
            });

            const AccelerationStructure topLevelAs = device.createAccelerationStructure(AccelerationStructureOptions{
                    .label = "TopLevelAS",
                    .type = AccelerationStructureType::TopLevel,
                    .flags = AccelerationStructureFlagBits::PreferFastTrace,
                    .geometryTypesAndCount = {
                            {
                                    .geometry = AccelerationStructureGeometryInstancesData{
                                            .data = {
                                                    AccelerationStructureGeometryInstance{
                                                            .flags = GeometryInstanceFlagBits::TriangleFacingCullDisable,
                                                            .accelerationStructure = bottomLevelAs,
                                                    },
                                            },
                                    },
                                    .maxPrimitiveCount = 1,
                            },
                    },
            });

            // THEN
            CHECK(bottomLevelAs.isValid());
            CHECK(topLevelAs.isValid());

            // WHEN
            bindGroup.update(BindGroupEntry{
                    .binding = 0,
                    .resource = AccelerationStructureBinding{
                            .accelerationStructure = topLevelAs,
                    },
            });

            // THEN -> expect not errors

            // Note: technically we would have to issue commands to build the acceleration structures first

            // WHEN -> tracing rays
            CommandRecorder commandRecorder = device.createCommandRecorder();
            RayTracingPassCommandRecorder recorder = commandRecorder.beginRayTracingPass();
            recorder.setPipeline(pipeline);
            recorder.setBindGroup(0, bindGroup);
            recorder.traceRays(RayTracingCommand{
                    .raygenShaderBindingTable = sbt.rayGenShaderRegion(),
                    .missShaderBindingTable = sbt.missShaderRegion(),
                    .hitShaderBindingTable = sbt.hitShaderRegion(),
                    .extent = { 1, 1, 1 },
            });
            recorder.end();
            CommandBuffer commandBuffer = commandRecorder.finish();

            // THEN
            CHECK(commandRecorder.isValid());
            CHECK(recorder.isValid());
            CHECK(commandBuffer.isValid());
        }
    }

#if VK_KHR_push_descriptor
    TEST_CASE("RayTracingPassCommandRecorder - pushBindGroup" * doctest::skip(!supportsRayTracing))
    {
        REQUIRE(discreteGPUAdapter->properties().pushBindGroupProperties.maxPushBindGroups > 0);

        // GIVEN
        auto rayTracingGenShader = device.createShaderModule(readShaderFile(rayTracingGenShaderPath));
        auto rayTracingMissShader = device.createShaderModule(readShaderFile(rayTracingMissShaderPath));
        auto rayTracingClosestShader = device.createShaderModule(readShaderFile(rayTracingClosestShaderPath));

        const Texture storageImage = device.createTexture(TextureOptions{
                .type = TextureType::TextureType2D,
                .format = Format::R8G8B8A8_UNORM,
                .extent = { 256, 256, 1 },
                .mipLevels = 1,
                .samples = SampleCountFlagBits::Samples1Bit,
                .usage = TextureUsageFlagBits::StorageBit | TextureUsageFlagBits::TransferDstBit,
                .memoryUsage = MemoryUsage::GpuOnly,
        });

        const TextureView storageImageView = storageImage.createView(TextureViewOptions{
                .viewType = ViewType::ViewType2D,
                .range = {
                        .aspectMask = TextureAspectFlagBits::ColorBit,
                },
        });

        const BindGroupLayoutOptions bindGroupLayoutOptions = {
            .bindings = {
                    {
                            .binding = 0,
                            .count = 1,
                            .resourceType = ResourceBindingType::AccelerationStructure,
                            .shaderStages = ShaderStageFlagBits::RaygenBit,
                    },
                    {
                            .binding = 1,
                            .count = 1,
                            .resourceType = ResourceBindingType::StorageImage,
                            .shaderStages = ShaderStageFlagBits::RaygenBit,
                    },
            },
            .flags = BindGroupLayoutFlagBits::PushBindGroup,
        };
        const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);

        const PipelineLayoutOptions pipelineLayoutOptions{
            .bindGroupLayouts = { bindGroupLayout },
        };
        const PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);

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
                    RayTracingShaderGroupOptions{
                            .type = RayTracingShaderGroupType::General,
                            .generalShaderIndex = 0,
                    },
                    RayTracingShaderGroupOptions{
                            .type = RayTracingShaderGroupType::General,
                            .generalShaderIndex = 1,
                    },
                    RayTracingShaderGroupOptions{
                            .type = RayTracingShaderGroupType::TrianglesHit,
                            .closestHitShaderIndex = 2,
                    },
            },
            .layout = pipelineLayout,
        };
        const RayTracingPipeline pipeline = device.createRayTracingPipeline(rayTracingPipelineOptions);

        REQUIRE(storageImage.isValid());
        REQUIRE(storageImageView.isValid());
        REQUIRE(bindGroupLayout.isValid());
        REQUIRE(pipelineLayout.isValid());
        REQUIRE(pipeline.isValid());

        // WHEN
        CommandRecorder commandRecorder = device.createCommandRecorder();
        RayTracingPassCommandRecorder recorder = commandRecorder.beginRayTracingPass();
        recorder.setPipeline(pipeline);
        recorder.pushBindGroup(0,
                               std::vector<BindGroupEntry>{
                                       BindGroupEntry{
                                               .binding = 1,
                                               .resource = ImageBinding{ .textureView = storageImageView },
                                       },
                               },
                               pipelineLayout);
        recorder.end();
        CommandBuffer commandBuffer = commandRecorder.finish();

        // THEN
        CHECK(commandRecorder.isValid());
        CHECK(recorder.isValid());
        CHECK(commandBuffer.isValid());
    }
#endif
}

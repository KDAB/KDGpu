/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/buffer.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/compute_pipeline.h>
#include <KDGpu/compute_pipeline_options.h>
#include <KDGpu/compute_pass_command_recorder.h>
#include <KDGpu/command_recorder.h>
#include <KDGpu/device.h>
#include <KDGpu/queue.h>
#include <KDGpu/instance.h>
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

Adapter *selectComputeAdapter(const Instance &instance)
{
    // Select Adapter that supports Compute
    for (Adapter *adapter : instance.adapters()) {
        const auto &queueTypes = adapter->queueTypes();
        for (const auto &queueType : queueTypes) {
            const bool hasCompute = queueType.supportsFeature(QueueFlags(QueueFlagBits::ComputeBit));
            if (hasCompute) {
                return adapter;
                break;
            }
        }
    }
    return nullptr;
}

} // namespace

TEST_SUITE("ComputePassCommandRecorder")
{
    // GIVEN
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "ComputePassCommandRecorder",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter *computeAdapter = selectComputeAdapter(instance);

    TEST_CASE("ComputePassCommandRecorder" * doctest::skip(computeAdapter == nullptr))
    {
        REQUIRE(computeAdapter);
        REQUIRE(computeAdapter->isValid());

        Device device = computeAdapter->createDevice();

        Queue computeQueue;
        const auto &queues = device.queues();
        for (const auto &q : queues) {
            if (q.flags() | QueueFlags(QueueFlagBits::ComputeBit)) {
                computeQueue = q;
                break;
            }
        }

        const auto computeShaderPath = assetPath() + "/shaders/tests/compute_pipeline/empty_compute.comp.spv";
        auto computeShader = device.createShaderModule(readShaderFile(computeShaderPath));

        // THEN
        REQUIRE(device.isValid());
        REQUIRE(computeQueue.isValid());
        REQUIRE(computeShader.isValid());

        SUBCASE("Can't be default constructed")
        {
            // EXPECT
            REQUIRE(!std::is_default_constructible<ComputePassCommandRecorder>::value);
            REQUIRE(!std::is_trivially_default_constructible<ComputePassCommandRecorder>::value);
        }

        SUBCASE("A constructed ComputePassCommandRecorder from a Vulkan API")
        {

            // GIVEN
            const PipelineLayoutOptions pipelineLayoutOptions{};
            const PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);
            const ComputePipelineOptions computePipelineOptions{
                .layout = pipelineLayout,
                .shaderStage = ComputeShaderStage{ .shaderModule = computeShader.handle() }
            };
            const ComputePipeline computePipeline = device.createComputePipeline(computePipelineOptions);

            // THEN
            CHECK(computePipeline.isValid());

            // WHEN
            const CommandRecorderOptions commandRecorderOptions{
                .queue = computeQueue
            };
            CommandRecorder commandRecorder = device.createCommandRecorder(commandRecorderOptions);

            // THEN
            CHECK(commandRecorder.isValid());

            // WHEN
            ComputePassCommandRecorder computeCommandRecorder = commandRecorder.beginComputePass();

            // THEN
            CHECK(computeCommandRecorder.isValid());
        }

        SUBCASE("Move constructor & move assignment")
        {
            // GIVEN
            const PipelineLayoutOptions pipelineLayoutOptions{};
            const PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);
            const ComputePipelineOptions computePipelineOptions{
                .layout = pipelineLayout,
                .shaderStage = ComputeShaderStage{ .shaderModule = computeShader.handle() }
            };
            const ComputePipeline computePipeline = device.createComputePipeline(computePipelineOptions);

            // THEN
            CHECK(computePipeline.isValid());

            // WHEN
            const CommandRecorderOptions commandRecorderOptions{
                .queue = computeQueue
            };
            CommandRecorder commandRecorder = device.createCommandRecorder(commandRecorderOptions);

            // THEN
            CHECK(commandRecorder.isValid());

            // WHEN
            ComputePassCommandRecorder computeCommandRecorder1 = commandRecorder.beginComputePass();
            ComputePassCommandRecorder computeCommandRecorder2(std::move(computeCommandRecorder1));

            // THEN
            CHECK(!computeCommandRecorder1.isValid());
            CHECK(computeCommandRecorder2.isValid());

            // WHEN
            ComputePassCommandRecorder computeCommandRecorder3 = commandRecorder.beginComputePass();
            const auto computeCommandRecorder2Handle = computeCommandRecorder2.handle();
            computeCommandRecorder3 = std::move(computeCommandRecorder2);

            // THEN
            CHECK(!computeCommandRecorder2.isValid());
            CHECK(computeCommandRecorder3.isValid());
            CHECK(computeCommandRecorder3.handle() == computeCommandRecorder2Handle);
        }

        SUBCASE("Destruction")
        {
            const PipelineLayoutOptions pipelineLayoutOptions{};
            const PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);
            const ComputePipelineOptions computePipelineOptions{
                .layout = pipelineLayout,
                .shaderStage = ComputeShaderStage{ .shaderModule = computeShader.handle() }
            };
            const ComputePipeline computePipeline = device.createComputePipeline(computePipelineOptions);

            // THEN
            CHECK(computePipeline.isValid());

            // WHEN
            const CommandRecorderOptions commandRecorderOptions{
                .queue = computeQueue
            };

            CommandRecorder commandRecorder = device.createCommandRecorder(commandRecorderOptions);
            Handle<ComputePassCommandRecorder_t> recorderHandle;

            {
                // WHEN
                ComputePassCommandRecorder computeCommandRecorder = commandRecorder.beginComputePass();
                recorderHandle = computeCommandRecorder.handle();

                // THEN
                CHECK(commandRecorder.isValid());
                CHECK(computeCommandRecorder.isValid());
                CHECK(recorderHandle.isValid());
                CHECK(api->resourceManager()->getComputePassCommandRecorder(recorderHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getComputePassCommandRecorder(recorderHandle) == nullptr);
        }

        SUBCASE("Dispatch Compute")
        {
            // GIVEN
            const PipelineLayoutOptions pipelineLayoutOptions{};
            const PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);
            const ComputePipelineOptions computePipelineOptions{
                .layout = pipelineLayout,
                .shaderStage = ComputeShaderStage{ .shaderModule = computeShader.handle() }
            };
            const ComputePipeline computePipeline = device.createComputePipeline(computePipelineOptions);

            // THEN
            CHECK(computePipeline.isValid());

            // WHEN
            const CommandRecorderOptions commandRecorderOptions{
                .queue = computeQueue
            };
            CommandRecorder commandRecorder = device.createCommandRecorder(commandRecorderOptions);

            // THEN
            CHECK(commandRecorder.isValid());

            // WHEN
            ComputePassCommandRecorder computeCommandRecorder = commandRecorder.beginComputePass();
            computeCommandRecorder.setPipeline(computePipeline);
            computeCommandRecorder.dispatchCompute(ComputeCommand{
                    .workGroupX = 1,
                    .workGroupY = 1,
                    .workGroupZ = 1,
            });
            computeCommandRecorder.dispatchCompute(std::vector<ComputeCommand>{
                    {
                            .workGroupX = 1,
                            .workGroupY = 1,
                            .workGroupZ = 1,
                    },
                    {
                            .workGroupX = 2,
                            .workGroupY = 2,
                            .workGroupZ = 2,
                    },
            });
            computeCommandRecorder.end();

            CommandBuffer commandBuffer = commandRecorder.finish();

            // THEN
            CHECK(commandRecorder.isValid());
            CHECK(computeCommandRecorder.isValid());
        }

        SUBCASE("Dispatch ComputeIndirect")
        {
            // GIVEN
            const PipelineLayoutOptions pipelineLayoutOptions{};
            const PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);
            const ComputePipelineOptions computePipelineOptions{
                .layout = pipelineLayout,
                .shaderStage = ComputeShaderStage{ .shaderModule = computeShader.handle() }
            };
            const ComputePipeline computePipeline = device.createComputePipeline(computePipelineOptions);

            // THEN
            CHECK(computePipeline.isValid());

            // WHEN
            const Buffer indirectComputeBuffer = device.createBuffer(BufferOptions{
                    .size = 2 * sizeof(ComputeCommand),
                    .usage = BufferUsageFlagBits::IndirectBufferBit | BufferUsageFlagBits::StorageBufferBit,
                    .memoryUsage = MemoryUsage::CpuToGpu,
            });

            // THEN
            CHECK(indirectComputeBuffer.isValid());

            // WHEN
            const CommandRecorderOptions commandRecorderOptions{
                .queue = computeQueue
            };
            CommandRecorder commandRecorder = device.createCommandRecorder(commandRecorderOptions);

            // THEN
            CHECK(commandRecorder.isValid());

            // WHEN
            ComputePassCommandRecorder computeCommandRecorder = commandRecorder.beginComputePass();
            computeCommandRecorder.setPipeline(computePipeline);
            computeCommandRecorder.dispatchComputeIndirect(ComputeCommandIndirect{
                    .buffer = indirectComputeBuffer,
                    .offset = 0,
            });
            computeCommandRecorder.dispatchComputeIndirect(std::vector<ComputeCommandIndirect>{
                    {
                            .buffer = indirectComputeBuffer,
                            .offset = 0,
                    },
                    {
                            .buffer = indirectComputeBuffer,
                            .offset = sizeof(ComputeCommand),
                    },
            });
            computeCommandRecorder.end();

            CommandBuffer commandBuffer = commandRecorder.finish();

            // THEN
            CHECK(commandRecorder.isValid());
            CHECK(computeCommandRecorder.isValid());
        }
    }

#if defined(VK_KHR_push_descriptor)
    TEST_CASE("PushBindGroup" * doctest::skip((computeAdapter == nullptr || !(computeAdapter->properties().pushBindGroupProperties.maxPushBindGroups > 0))))
    {
        REQUIRE(computeAdapter);
        REQUIRE(computeAdapter->isValid());

        Device device = computeAdapter->createDevice();

        Queue computeQueue;
        const auto &queues = device.queues();
        for (const auto &q : queues) {
            if (q.flags() | QueueFlags(QueueFlagBits::ComputeBit)) {
                computeQueue = q;
                break;
            }
        }

        // THEN
        REQUIRE(device.isValid());
        REQUIRE(computeQueue.isValid());

        // GIVEN
        const Buffer storageBuffer = device.createBuffer(BufferOptions{
                .size = 64, // 64 bytes for a 4x4 matrix
                .usage = BufferUsageFlagBits::StorageBufferBit,
                .memoryUsage = MemoryUsage::CpuToGpu,
        });

        // Create bind group layout for the push descriptor test
        const BindGroupLayoutOptions bindGroupLayoutOptions = {
            .bindings = {
                    { .binding = 0, .resourceType = ResourceBindingType::StorageBuffer, .shaderStages = ShaderStageFlags(ShaderStageFlagBits::ComputeBit) },
            },
            .flags = BindGroupLayoutFlagBits::PushBindGroup, // Enable push descriptor
        };
        const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);

        // Create pipeline layout with the bind group layout
        const PipelineLayoutOptions pipelineLayoutOptions = {
            .bindGroupLayouts = { bindGroupLayout },
        };
        const PipelineLayout pushBindGroupPipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);

        // Create a pipeline that uses the bind group layout
        const auto computeShaderPath = assetPath() + "/shaders/tests/compute_pipeline/empty_compute_with_bindgroup.comp.spv";
        auto pushBindGroupComputeShader = device.createShaderModule(readShaderFile(computeShaderPath));

        const ComputePipelineOptions computePipelineOptions{
            .layout = pushBindGroupPipelineLayout,
            .shaderStage = ComputeShaderStage{ .shaderModule = pushBindGroupComputeShader }
        };
        const ComputePipeline computePipeline = device.createComputePipeline(computePipelineOptions);

        // THEN
        CHECK(computePipeline.isValid());
        REQUIRE(storageBuffer.isValid());
        REQUIRE(bindGroupLayout.isValid());
        REQUIRE(pushBindGroupPipelineLayout.isValid());
        REQUIRE(pushBindGroupComputeShader.isValid());

        // WHEN
        CommandRecorder commandRecorder = device.createCommandRecorder(CommandRecorderOptions{
                .queue = computeQueue,
        });

        // THEN
        CHECK(commandRecorder.isValid());

        // WHEN
        ComputePassCommandRecorder computeCommandRecorder = commandRecorder.beginComputePass();

        // Test pushBindGroup functionality
        computeCommandRecorder.setPipeline(computePipeline);

        // Push the bind group
        computeCommandRecorder.pushBindGroup(0,
                                             std::vector<BindGroupEntry>{
                                                     BindGroupEntry{
                                                             .binding = 0,
                                                             .resource = StorageBufferBinding{ .buffer = storageBuffer },
                                                     },
                                             },
                                             pushBindGroupPipelineLayout);

        computeCommandRecorder.end();
        CommandBuffer commandBuffer = commandRecorder.finish();

        // THEN
        CHECK(commandRecorder.isValid());
        CHECK(computeCommandRecorder.isValid());
        CHECK(storageBuffer.isValid());
        CHECK(bindGroupLayout.isValid());
        CHECK(pushBindGroupPipelineLayout.isValid());
        CHECK(computePipeline.isValid());
        CHECK(commandBuffer.isValid());
    }
#endif
}

/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/timestamp_query_recorder.h>
#include <KDGpu/command_recorder.h>
#include <KDGpu/device.h>
#include <KDGpu/queue.h>
#include <KDGpu/instance.h>
#include <KDGpu/device_options.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/buffer.h>

#include <type_traits>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDGpu;

TEST_SUITE("TimestampQueryRecorder")
{
    // GIVEN
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "TimestampQueryRecorder",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });

    Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);

    TEST_CASE("TimestampQueryRecorder")
    {
        Device device = discreteGPUAdapter->createDevice();
        Queue &transferQueue = device.queues()[0];

        // THEN
        REQUIRE(device.isValid());

        SUBCASE("Can be default constructed")
        {
            // EXPECT
            REQUIRE(std::is_default_constructible<TimestampQueryRecorder>::value);
            REQUIRE(!std::is_trivially_default_constructible<TimestampQueryRecorder>::value);
        }

        SUBCASE("A constructed TimestampQueryRecorder from a Vulkan API")
        {
            // GIVEN
            CommandRecorder commandRecorder = device.createCommandRecorder();

            TimestampQueryRecorder timestampQueryRecorder = commandRecorder.beginTimestampRecording(TimestampQueryRecorderOptions{
                    .queryCount = 2,
            });

            CommandBuffer commandBuffer = commandRecorder.finish();

            // THEN
            CHECK(commandRecorder.isValid());
            CHECK(timestampQueryRecorder.isValid());
        }

        SUBCASE("Move constructor & move assignment")
        {
            // GIVEN
            CommandRecorder commandRecorder = device.createCommandRecorder();

            TimestampQueryRecorder timestampQueryRecorder1 = commandRecorder.beginTimestampRecording(TimestampQueryRecorderOptions{
                    .queryCount = 2,
            });
            // WHEN
            TimestampQueryRecorder timestampQueryRecorder2(std::move(timestampQueryRecorder1));

            // THEN
            CHECK(!timestampQueryRecorder1.isValid());
            CHECK(timestampQueryRecorder2.isValid());

            // WHEN
            TimestampQueryRecorder timestampQueryRecorder3 = commandRecorder.beginTimestampRecording(TimestampQueryRecorderOptions{
                    .queryCount = 2,
            });
            const auto timestampQueryRecorder2Handle = timestampQueryRecorder2.handle();
            timestampQueryRecorder3 = std::move(timestampQueryRecorder2);

            // THEN
            CHECK(!timestampQueryRecorder2.isValid());
            CHECK(timestampQueryRecorder3.isValid());
            CHECK(timestampQueryRecorder3.handle() == timestampQueryRecorder2Handle);
        }

        SUBCASE("Can record timestamps")
        {

            // GIVEN
            const BufferOptions cpuGpuBufferOptions = {
                .label = "cpuGpu",
                .size = 4 * sizeof(float),
                .usage = BufferUsageFlagBits::TransferSrcBit,
                .memoryUsage = MemoryUsage::CpuToGpu
            };
            const BufferOptions gpuGpuBufferOptions = {
                .label = "gpuGpu",
                .size = 1024 * 1024 * sizeof(float),
                .usage = BufferUsageFlagBits::TransferSrcBit | BufferUsageFlagBits::TransferDstBit,
                .memoryUsage = MemoryUsage::GpuOnly
            };
            const BufferOptions gpuCpuBufferOptions = {
                .label = "gpuCpu",
                .size = 4 * sizeof(float),
                .usage = BufferUsageFlagBits::TransferSrcBit | BufferUsageFlagBits::TransferDstBit,
                .memoryUsage = MemoryUsage::GpuToCpu
            };

            // WHEN
            const float initialData[] = { 1.0f, 2.0f, 3.0f, 4.0f };
            Buffer cpuToGpu = device.createBuffer(cpuGpuBufferOptions, initialData);
            Buffer gpuToGpu = device.createBuffer(gpuGpuBufferOptions);
            Buffer gpuToCpu = device.createBuffer(gpuCpuBufferOptions);

            // THEN
            CHECK(cpuToGpu.isValid());
            CHECK(gpuToGpu.isValid());
            CHECK(gpuToCpu.isValid());

            // WHEN
            CommandRecorder commandRecorder = device.createCommandRecorder();

            TimestampQueryRecorder timestampQueryRecorder = commandRecorder.beginTimestampRecording(TimestampQueryRecorderOptions{
                    .queryCount = 6,
            });

            // THEN
            CHECK(commandRecorder.isValid());
            CHECK(timestampQueryRecorder.isValid());

            // WHEN
            TimestampIndex t0 = timestampQueryRecorder.writeTimestamp(PipelineStageFlagBit::TopOfPipeBit);
            // Copy cpuGpu[2], cpuGpu[3] -> gpuGpu[0], gpuGpu[1]
            commandRecorder.copyBuffer(BufferCopy{
                    .src = cpuToGpu,
                    .srcOffset = 2 * sizeof(float),
                    .dst = gpuToGpu,
                    .dstOffset = 0,
                    .byteSize = 2 * sizeof(float) });
            TimestampIndex t1 = timestampQueryRecorder.writeTimestamp(PipelineStageFlagBit::BottomOfPipeBit);

            TimestampIndex t2 = timestampQueryRecorder.writeTimestamp(PipelineStageFlagBit::TopOfPipeBit);
            // Copy cpuGpu[0], cpuGpu[1] -> gpuGpu[2], gpuGpu[3]
            commandRecorder.copyBuffer(BufferCopy{
                    .src = cpuToGpu,
                    .srcOffset = 0,
                    .dst = gpuToGpu,
                    .dstOffset = 2 * sizeof(float),
                    .byteSize = 2 * sizeof(float) });
            TimestampIndex t3 = timestampQueryRecorder.writeTimestamp(PipelineStageFlagBit::BottomOfPipeBit);

            TimestampIndex t4 = timestampQueryRecorder.writeTimestamp(PipelineStageFlagBit::TopOfPipeBit);
            commandRecorder.bufferMemoryBarrier(BufferMemoryBarrierOptions{
                    .srcStages = PipelineStageFlagBit::CopyBit | PipelineStageFlagBit::TransferBit,
                    .srcMask = AccessFlagBit::MemoryWriteBit,
                    .dstStages = PipelineStageFlagBit::ClearBit,
                    .dstMask = AccessFlagBit::MemoryWriteBit,
                    .buffer = gpuToGpu,
            });

            // Clear gpuGpu
            commandRecorder.clearBuffer(BufferClear{
                    .dstBuffer = gpuToGpu,
                    .dstOffset = 0,
                    .byteSize = gpuGpuBufferOptions.size,
            });
            TimestampIndex t5 = timestampQueryRecorder.writeTimestamp(PipelineStageFlagBit::BottomOfPipeBit);

            // THEN
            CommandBuffer commandBuffer = commandRecorder.finish();

            transferQueue.submit(SubmitOptions{
                    .commandBuffers = { commandBuffer } });

            device.waitUntilIdle();

            const std::vector<uint64_t> results = timestampQueryRecorder.queryResults();

            CHECK(results.size() == 6);

            SPDLOG_WARN("Timestamps T0: {}", results[t0]);
            SPDLOG_WARN("Timestamps T1: {}", results[t1]);
            SPDLOG_WARN("Timestamps T2: {}", results[t2]);
            SPDLOG_WARN("Timestamps T3: {}", results[t3]);
            SPDLOG_WARN("Timestamps T4: {}", results[t4]);
            SPDLOG_WARN("Timestamps T5: {}", results[t5]);

            SPDLOG_WARN("Interval T0 - T1: {} ns", timestampQueryRecorder.nsInterval(t0, t1));
            SPDLOG_WARN("Interval T2 - T3: {} ns", timestampQueryRecorder.nsInterval(t2, t3));
            SPDLOG_WARN("Interval T0 - T3: {} ns", timestampQueryRecorder.nsInterval(t0, t3));

            SPDLOG_WARN("Interval T4 - T5: {} ns", timestampQueryRecorder.nsInterval(t4, t5));
            SPDLOG_WARN("Interval T0 - T5: {} ns", timestampQueryRecorder.nsInterval(t0, t5));
        }

        SUBCASE("Warns and override last timestamp when not enough queries")
        {
            // GIVEN
            CommandRecorder commandRecorder = device.createCommandRecorder();

            TimestampQueryRecorder timestampQueryRecorder = commandRecorder.beginTimestampRecording(TimestampQueryRecorderOptions{
                    .queryCount = 2,
            });

            // THEN
            CHECK(commandRecorder.isValid());
            CHECK(timestampQueryRecorder.isValid());

            // WHEN
            TimestampIndex t0 = timestampQueryRecorder.writeTimestamp(PipelineStageFlagBit::BottomOfPipeBit);
            TimestampIndex t1 = timestampQueryRecorder.writeTimestamp(PipelineStageFlagBit::BottomOfPipeBit);

            // THEN
            CHECK(t0 != t1);

            // WHEN
            TimestampIndex t2 = timestampQueryRecorder.writeTimestamp(PipelineStageFlagBit::BottomOfPipeBit);

            // THEN
            CHECK(t1 == t2);

            CommandBuffer commandBuffer = commandRecorder.finish();
        }

        SUBCASE("Destruction")
        {
            // GIVEN
            Handle<TimestampQueryRecorder_t> recorderHandle;

            {
                // WHEN
                CommandRecorder commandRecorder = device.createCommandRecorder();
                TimestampQueryRecorder timestampQueryRecorder = commandRecorder.beginTimestampRecording(TimestampQueryRecorderOptions{
                        .queryCount = 2,
                });
                recorderHandle = timestampQueryRecorder.handle();

                CommandBuffer commandBuffer = commandRecorder.finish();

                // THEN
                CHECK(commandRecorder.isValid());
                CHECK(timestampQueryRecorder.isValid());
                CHECK(api->resourceManager()->getTimestampQueryRecorder(recorderHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getTimestampQueryRecorder(recorderHandle) == nullptr);
        }

        SUBCASE("Check doesn't run out of queries")
        {
            // GIVEN
            CommandRecorder commandRecorder = device.createCommandRecorder();

            // Note: we can allocate up to 1024 queries, that's a hard coded limit (see VulkanResourceManager::createTimestampQueryRecorder)

            // WHEN
            {
                TimestampQueryRecorder t1 = commandRecorder.beginTimestampRecording(TimestampQueryRecorderOptions{
                        .queryCount = 256,
                });
                TimestampQueryRecorder t2 = commandRecorder.beginTimestampRecording(TimestampQueryRecorderOptions{
                        .queryCount = 256,
                });
                TimestampQueryRecorder t3 = commandRecorder.beginTimestampRecording(TimestampQueryRecorderOptions{
                        .queryCount = 256,
                });
                TimestampQueryRecorder t4 = commandRecorder.beginTimestampRecording(TimestampQueryRecorderOptions{
                        .queryCount = 256,
                });
            }

            // THEN -> No Validation error

            // WHEN
            {
                TimestampQueryRecorder t1 = commandRecorder.beginTimestampRecording(TimestampQueryRecorderOptions{
                        .queryCount = 256,
                });
                TimestampQueryRecorder t2 = commandRecorder.beginTimestampRecording(TimestampQueryRecorderOptions{
                        .queryCount = 256,
                });
                TimestampQueryRecorder t3 = commandRecorder.beginTimestampRecording(TimestampQueryRecorderOptions{
                        .queryCount = 256,
                });
                TimestampQueryRecorder t4 = commandRecorder.beginTimestampRecording(TimestampQueryRecorderOptions{
                        .queryCount = 256,
                });

                t1 = {};

                TimestampQueryRecorder t5 = commandRecorder.beginTimestampRecording(TimestampQueryRecorderOptions{
                        .queryCount = 256,
                });

                t4 = {};
                t3 = {};

                TimestampQueryRecorder t6 = commandRecorder.beginTimestampRecording(TimestampQueryRecorderOptions{
                        .queryCount = 512,
                });
            }

            // THEN -> No Validation error
        }
    }
}

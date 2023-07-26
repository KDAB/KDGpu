/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/command_recorder.h>
#include <KDGpu/device.h>
#include <KDGpu/instance.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/buffer.h>
#include <KDGpu/texture_options.h>
#include <KDGpu/texture.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

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
} // namespace

TEST_CASE("CommandRecorder")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "CommandRecorder",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });

    Adapter *transferAdapter = nullptr;
    // Select Adapter that supports Transfers
    for (auto &adapter : instance.adapters()) {
        const auto &queueTypes = adapter->queueTypes();
        for (const auto &queueType : queueTypes) {
            const bool hasTransfer = queueType.supportsFeature(QueueFlags(QueueFlagBits::TransferBit));
            if (hasTransfer) {
                transferAdapter = adapter;
                break;
            }
        }
    }
    REQUIRE(transferAdapter);
    REQUIRE(transferAdapter->isValid());

    Device device = transferAdapter->createDevice();

    Queue transferQueue;
    const auto &queues = device.queues();
    for (const auto &q : queues) {
        if (q.flags() | QueueFlags(QueueFlagBits::TransferBit)) {
            transferQueue = q;
            break;
        }
    }

    REQUIRE(device.isValid());
    REQUIRE(transferQueue.isValid());

    SUBCASE("A default constructed CommandRecorder is invalid")
    {
        REQUIRE(!std::is_default_constructible<CommandRecorder>::value);
        REQUIRE(!std::is_trivially_default_constructible<CommandRecorder>::value);
    }

    SUBCASE("A constructed ComputePipeline from a Vulkan API")
    {
        // GIVEN
        CommandRecorder c = device.createCommandRecorder();

        // THEN
        CHECK(c.isValid());
    }

    SUBCASE("Buffer Copies No Barriers")
    {
        // GIVEN
        const BufferOptions cpuGpuBufferOptions = {
            .size = 4 * sizeof(float),
            .usage = BufferUsageFlagBits::TransferSrcBit,
            .memoryUsage = MemoryUsage::CpuToGpu
        };
        const BufferOptions gpuGpuBufferOptions = {
            .size = 4 * sizeof(float),
            .usage = BufferUsageFlagBits::TransferSrcBit | BufferUsageFlagBits::TransferDstBit,
            .memoryUsage = MemoryUsage::GpuOnly
        };
        const BufferOptions gpuCpuBufferOptions = {
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

        {
            // WHEN
            const float *m = reinterpret_cast<const float *>(cpuToGpu.map());

            // THEN
            CHECK(m != nullptr);
            CHECK(m[0] == initialData[0]);
            CHECK(m[1] == initialData[1]);
            CHECK(m[2] == initialData[2]);
            CHECK(m[3] == initialData[3]);

            // WHEN
            cpuToGpu.unmap();
        }

        // WHEN
        {
            CommandRecorder c = device.createCommandRecorder();

            // Copy cpuGpu[2], cpuGpu[3] -> gpuGpu[0], gpuGpu[1]
            c.copyBuffer(BufferCopy{
                    .src = cpuToGpu,
                    .srcOffset = 2 * sizeof(float),
                    .dst = gpuToGpu,
                    .dstOffset = 0,
                    .byteSize = 2 * sizeof(float) });

            auto commandBuffer = c.finish();

            transferQueue.submit(SubmitOptions{
                    .commandBuffers = { commandBuffer } });

            device.waitUntilIdle();
        }

        {
            CommandRecorder c = device.createCommandRecorder();

            // Copy cpuGpu[0], cpuGpu[1] -> gpuGpu[2], gpuGpu[3]
            c.copyBuffer(BufferCopy{
                    .src = cpuToGpu,
                    .srcOffset = 0,
                    .dst = gpuToGpu,
                    .dstOffset = 2 * sizeof(float),
                    .byteSize = 2 * sizeof(float) });

            auto commandBuffer = c.finish();

            transferQueue.submit(SubmitOptions{
                    .commandBuffers = { commandBuffer } });

            device.waitUntilIdle();
        }

        {
            CommandRecorder c = device.createCommandRecorder();

            // Copy gpuGpu to gpuCpu
            c.copyBuffer(BufferCopy{
                    .src = gpuToGpu,
                    .srcOffset = 0,
                    .dst = gpuToCpu,
                    .dstOffset = 0,
                    .byteSize = 4 * sizeof(float) });

            auto commandBuffer = c.finish();

            transferQueue.submit(SubmitOptions{
                    .commandBuffers = { commandBuffer } });

            device.waitUntilIdle();
        }

        // THEN
        {
            // WHEN
            const float *m = reinterpret_cast<const float *>(gpuToCpu.map());

            // THEN
            CHECK(m != nullptr);
            CHECK(m[0] == initialData[2]);
            CHECK(m[1] == initialData[3]);
            CHECK(m[2] == initialData[0]);
            CHECK(m[3] == initialData[1]);

            // WHEN
            gpuToCpu.unmap();
        }
    }

    SUBCASE("Buffer Copies Barriers")
    {
        // GIVEN
        const BufferOptions cpuGpuBufferOptions = {
            .size = 4 * sizeof(float),
            .usage = BufferUsageFlagBits::TransferSrcBit,
            .memoryUsage = MemoryUsage::CpuToGpu
        };
        const BufferOptions gpuGpuBufferOptions = {
            .size = 4 * sizeof(float),
            .usage = BufferUsageFlagBits::TransferSrcBit | BufferUsageFlagBits::TransferDstBit,
            .memoryUsage = MemoryUsage::GpuOnly
        };
        const BufferOptions gpuCpuBufferOptions = {
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

        {
            // WHEN
            const float *m = reinterpret_cast<const float *>(cpuToGpu.map());

            // THEN
            CHECK(m != nullptr);
            CHECK(m[0] == initialData[0]);
            CHECK(m[1] == initialData[1]);
            CHECK(m[2] == initialData[2]);
            CHECK(m[3] == initialData[3]);

            // WHEN
            cpuToGpu.unmap();
        }

        // WHEN
        CommandRecorder c = device.createCommandRecorder();

        // Copy cpuGpu[2], cpuGpu[3] -> gpuGpu[0], gpuGpu[1]
        c.copyBuffer(BufferCopy{
                .src = cpuToGpu,
                .srcOffset = 2 * sizeof(float),
                .dst = gpuToGpu,
                .dstOffset = 0,
                .byteSize = 2 * sizeof(float) });

        // Copy cpuGpu[0], cpuGpu[1] -> gpuGpu[2], gpuGpu[3]
        c.copyBuffer(BufferCopy{
                .src = cpuToGpu,
                .srcOffset = 0,
                .dst = gpuToGpu,
                .dstOffset = 2 * sizeof(float),
                .byteSize = 2 * sizeof(float) });

        // Memory Barrier to ensure all writes to gpuToGpu are completed before
        // commands to follow are executed
        // clang-format off
        c.memoryBarrier(MemoryBarrierOptions {
                .srcStages = PipelineStageFlags(PipelineStageFlagBit::TransferBit),
                .dstStages = PipelineStageFlags(PipelineStageFlagBit::TransferBit),
                .memoryBarriers = {
                            {
                                .srcMask = AccessFlags(AccessFlagBit::TransferReadBit) | AccessFlags(AccessFlagBit::TransferWriteBit),
                                .dstMask = AccessFlags(AccessFlagBit::TransferReadBit) | AccessFlags(AccessFlagBit::TransferWriteBit)
                            }
                }
        });
        // clang-format on

        // Copy gpuGpu to gpuCpu
        c.copyBuffer(BufferCopy{
                .src = gpuToGpu,
                .srcOffset = 0,
                .dst = gpuToCpu,
                .dstOffset = 0,
                .byteSize = 4 * sizeof(float) });

        auto commandBuffer = c.finish();

        transferQueue.submit(SubmitOptions{
                .commandBuffers = { commandBuffer } });

        device.waitUntilIdle();

        // THEN
        {
            // WHEN
            const float *m = reinterpret_cast<const float *>(gpuToCpu.map());

            // THEN
            CHECK(m != nullptr);
            CHECK(m[0] == initialData[2]);
            CHECK(m[1] == initialData[3]);
            CHECK(m[2] == initialData[0]);
            CHECK(m[3] == initialData[1]);

            // WHEN
            gpuToCpu.unmap();
        }
    }

    SUBCASE("Execute Secondary Command Buffer")
    {
        // GIVEN
        const BufferOptions cpuGpuBufferOptions = {
            .size = 4 * sizeof(float),
            .usage = BufferUsageFlagBits::TransferSrcBit,
            .memoryUsage = MemoryUsage::CpuToGpu
        };
        const BufferOptions gpuCpuBufferOptions = {
            .size = 4 * sizeof(float),
            .usage = BufferUsageFlagBits::TransferSrcBit | BufferUsageFlagBits::TransferDstBit,
            .memoryUsage = MemoryUsage::GpuToCpu
        };
        const float initialData[] = { 1.0f, 2.0f, 3.0f, 4.0f };
        Buffer cpuToGpu = device.createBuffer(cpuGpuBufferOptions, initialData);
        Buffer gpuToCpu = device.createBuffer(gpuCpuBufferOptions);

        CHECK(cpuToGpu.isValid());
        CHECK(gpuToCpu.isValid());

        CommandRecorder primaryCommandRecorder = device.createCommandRecorder();

        // WHEN

        // 1) Record Commands Into a secondary CommandBuffer
        CommandRecorder secondaryCommandRecorder = device.createCommandRecorder(CommandRecorderOptions{ .level = CommandBufferLevel::Secondary });

        secondaryCommandRecorder.copyBuffer(BufferCopy{
                .src = cpuToGpu,
                .srcOffset = 0,
                .dst = gpuToCpu,
                .dstOffset = 0,
                .byteSize = 4 * sizeof(float) });

        CommandBuffer secondaryCommandBuffer = secondaryCommandRecorder.finish();

        // 2) Execute SecondaryCommand Buffer on Primary CommandBuffer
        primaryCommandRecorder.executeSecondaryCommandBuffer(secondaryCommandBuffer);

        // 3) Submit Primary Command Buffer and Wait for Completion
        CommandBuffer primaryCommandBuffer = primaryCommandRecorder.finish();

        transferQueue.submit(SubmitOptions{
                .commandBuffers = { primaryCommandBuffer } });

        device.waitUntilIdle();

        // THEN
        const float *m = reinterpret_cast<const float *>(gpuToCpu.map());

        CHECK(m != nullptr);
        CHECK(m[0] == initialData[0]);
        CHECK(m[1] == initialData[1]);
        CHECK(m[2] == initialData[2]);
        CHECK(m[3] == initialData[3]);

        gpuToCpu.unmap();
    }

    SUBCASE("Blit Texture")
    {
        // GIVEN
        const TextureOptions textureOptions = {
            .type = TextureType::TextureType2D,
            .format = Format::R8G8B8A8_SNORM,
            .extent = { 512, 512, 1 },
            .mipLevels = 2,
            .usage = TextureUsageFlagBits::TransferSrcBit | TextureUsageFlagBits::TransferDstBit,
            .memoryUsage = MemoryUsage::GpuOnly,
            .initialLayout = TextureLayout::Undefined,
        };
        Texture t = device.createTexture(textureOptions);

        // WEN
        CommandRecorder primaryCommandRecorder = device.createCommandRecorder();

        // Transition base miplevel to TransferSrcOptimal
        primaryCommandRecorder.textureMemoryBarrier(TextureMemoryBarrierOptions{
                .srcStages = PipelineStageFlagBit::TransferBit,
                .srcMask = AccessFlagBit::None,
                .dstStages = PipelineStageFlagBit::TransferBit,
                .dstMask = AccessFlagBit::TransferReadBit,
                .oldLayout = TextureLayout::Undefined,
                .newLayout = TextureLayout::TransferSrcOptimal,
                .texture = t,
                .range = {
                        .aspectMask = TextureAspectFlagBits::ColorBit,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                },
        });

        // Transition first miplevel to TransforDstOptimal
        primaryCommandRecorder.textureMemoryBarrier(TextureMemoryBarrierOptions{
                .srcStages = PipelineStageFlagBit::TransferBit,
                .srcMask = AccessFlagBit::None,
                .dstStages = PipelineStageFlagBit::TransferBit,
                .dstMask = AccessFlagBit::TransferWriteBit,
                .oldLayout = TextureLayout::Undefined,
                .newLayout = TextureLayout::TransferDstOptimal,
                .texture = t,
                .range = {
                        .aspectMask = TextureAspectFlagBits::ColorBit,
                        .baseMipLevel = 1,
                        .levelCount = 1,
                },
        });

        primaryCommandRecorder.blitTexture(TextureBlitOptions{
                .srcTexture = t,
                .srcLayout = TextureLayout::TransferSrcOptimal,
                .dstTexture = t,
                .dstLayout = TextureLayout::TransferDstOptimal,
                .regions = {
                        {
                                .srcSubresource = {
                                        .aspectMask = TextureAspectFlagBits::ColorBit,
                                        .mipLevel = 0,
                                },
                                .srcOffset = {
                                        .x = 0,
                                        .y = 0,
                                        .z = 0,
                                },
                                .srcExtent = {
                                        .width = 512,
                                        .height = 512,
                                        .depth = 1,
                                },
                                .dstSubresource = {
                                        .aspectMask = TextureAspectFlagBits::ColorBit,
                                        .mipLevel = 1,
                                },
                                .dstOffset = {
                                        .x = 0,
                                        .y = 0,
                                        .z = 0,
                                },
                                .dstExtent = {
                                        .width = 256,
                                        .height = 256,
                                        .depth = 1,
                                },
                        },
                },
                .scalingFilter = FilterMode::Nearest,
        });

        auto commandBuffer = primaryCommandRecorder.finish();

        transferQueue.submit(SubmitOptions{
                .commandBuffers = { commandBuffer } });

        device.waitUntilIdle();

        // THEN -> Shouldn't log validation errors
    }

    SUBCASE("Resolve Texture")
    {
        // GIVEN
        Texture tMSAA = device.createTexture(TextureOptions{
                .type = TextureType::TextureType2D,
                .format = Format::R8G8B8A8_SNORM,
                .extent = { 512, 512, 1 },
                .mipLevels = 1,
                .samples = SampleCountFlagBits::Samples4Bit,
                .usage = TextureUsageFlagBits::TransferSrcBit | TextureUsageFlagBits::TransferDstBit,
                .memoryUsage = MemoryUsage::GpuOnly,
                .initialLayout = TextureLayout::Undefined,
        });
        Texture tResolve = device.createTexture(TextureOptions{
                .type = TextureType::TextureType2D,
                .format = Format::R8G8B8A8_SNORM,
                .extent = { 512, 512, 1 },
                .mipLevels = 1,
                .samples = SampleCountFlagBits::Samples1Bit,
                .usage = TextureUsageFlagBits::TransferSrcBit | TextureUsageFlagBits::TransferDstBit,
                .memoryUsage = MemoryUsage::GpuOnly,
                .initialLayout = TextureLayout::Undefined,
        });

        // WEN
        CommandRecorder primaryCommandRecorder = device.createCommandRecorder();

        // Transition tMSAA to TransferSrcOptimal
        primaryCommandRecorder.textureMemoryBarrier(TextureMemoryBarrierOptions{
                .srcStages = PipelineStageFlagBit::TransferBit,
                .srcMask = AccessFlagBit::None,
                .dstStages = PipelineStageFlagBit::TransferBit,
                .dstMask = AccessFlagBit::TransferReadBit,
                .oldLayout = TextureLayout::Undefined,
                .newLayout = TextureLayout::TransferSrcOptimal,
                .texture = tMSAA,
                .range = {
                        .aspectMask = TextureAspectFlagBits::ColorBit,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                },
        });

        // Transition tResolve TransforDstOptimal
        primaryCommandRecorder.textureMemoryBarrier(TextureMemoryBarrierOptions{
                .srcStages = PipelineStageFlagBit::TransferBit,
                .srcMask = AccessFlagBit::None,
                .dstStages = PipelineStageFlagBit::TransferBit,
                .dstMask = AccessFlagBit::TransferWriteBit,
                .oldLayout = TextureLayout::Undefined,
                .newLayout = TextureLayout::TransferDstOptimal,
                .texture = tResolve,
                .range = {
                        .aspectMask = TextureAspectFlagBits::ColorBit,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                },
        });

        primaryCommandRecorder.resolveTexture(TextureResolveOptions{
                .srcTexture = tMSAA,
                .srcLayout = TextureLayout::TransferSrcOptimal,
                .dstTexture = tResolve,
                .dstLayout = TextureLayout::TransferDstOptimal,
                .regions = {
                        {
                                .srcSubresource = {
                                        .aspectMask = TextureAspectFlagBits::ColorBit,
                                        .mipLevel = 0,
                                },
                                .srcOffset = {
                                        .x = 0,
                                        .y = 0,
                                        .z = 0,
                                },
                                .dstSubresource = {
                                        .aspectMask = TextureAspectFlagBits::ColorBit,
                                        .mipLevel = 0,
                                },
                                .dstOffset = {
                                        .x = 0,
                                        .y = 0,
                                        .z = 0,
                                },
                                .extent = {
                                        .width = 512,
                                        .height = 512,
                                        .depth = 1,
                                },
                        },
                },
        });

        auto commandBuffer = primaryCommandRecorder.finish();

        transferQueue.submit(SubmitOptions{
                .commandBuffers = { commandBuffer } });

        device.waitUntilIdle();

        // THEN -> Shouldn't log validation errors
    }

    SUBCASE("Destruction - Going Out of Scope")
    {
        Handle<CommandRecorder_t> recorderHandle;

        {
            // WHEN
            CommandRecorder commandRecorder = device.createCommandRecorder();
            recorderHandle = commandRecorder.handle();

            // THEN
            CHECK(commandRecorder.isValid());
            CHECK(recorderHandle.isValid());
            CHECK(api->resourceManager()->getCommandRecorder(recorderHandle) != nullptr);
        }

        // THEN
        CHECK(api->resourceManager()->getCommandRecorder(recorderHandle) == nullptr);
    }

    SUBCASE("Destruction - Move assignment")
    {
        Handle<CommandRecorder_t> recorderHandle;

        // WHEN
        CommandRecorder commandRecorder = device.createCommandRecorder();
        recorderHandle = commandRecorder.handle();

        // THEN
        CHECK(commandRecorder.isValid());
        CHECK(recorderHandle.isValid());
        CHECK(api->resourceManager()->getCommandRecorder(recorderHandle) != nullptr);

        // WHEN
        commandRecorder = device.createCommandRecorder();

        // THEN
        CHECK(api->resourceManager()->getCommandRecorder(recorderHandle) == nullptr);
    }
}

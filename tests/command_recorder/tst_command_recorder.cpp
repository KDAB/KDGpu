#include <toy_renderer/command_recorder.h>
#include <toy_renderer/device.h>
#include <toy_renderer/instance.h>
#include <toy_renderer/buffer_options.h>
#include <toy_renderer/buffer.h>
#include <toy_renderer/vulkan/vulkan_graphics_api.h>

#include <type_traits>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace ToyRenderer;

namespace {
inline std::string assetPath()
{
#if defined(TOY_RENDERER_ASSET_PATH)
    return TOY_RENDERER_ASSET_PATH;
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
            .applicationVersion = SERENITY_MAKE_API_VERSION(0, 1, 0, 0) });

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

    SUBCASE("Destruction - Move assigmnent")
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

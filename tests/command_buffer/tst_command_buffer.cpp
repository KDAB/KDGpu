#include <toy_renderer/command_buffer.h>
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

TEST_CASE("CommandBuffer")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "CommandBuffer",
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

    SUBCASE("A default constructed CommandBuffer is invalid")
    {
        // GIVEN
        CommandBuffer cb;

        // THEN
        CHECK(!cb.isValid());
    }

    SUBCASE("A constructed CommandBuffer from a Vulkan API")
    {
        // GIVEN
        CommandRecorder recorder = device.createCommandRecorder();
        CommandBuffer cb = recorder.finish();

        // THEN
        CHECK(recorder.isValid());
        CHECK(cb.isValid());
    }

    SUBCASE("Destruction - Going Out of Scope")
    {
        // GIVEN
        CommandRecorder commandRecorder = device.createCommandRecorder();
        Handle<CommandBuffer_t> cbHandle;

        {
            // WHEN
            CommandBuffer commandBuffer = commandRecorder.finish();
            cbHandle = commandBuffer.handle();

            // THEN
            CHECK(commandRecorder.isValid());
            CHECK(commandBuffer.isValid());
            CHECK(cbHandle.isValid());
            CHECK(api->resourceManager()->getCommandBuffer(cbHandle) != nullptr);
        }

        // THEN
        CHECK(api->resourceManager()->getCommandBuffer(cbHandle) == nullptr);
    }

    SUBCASE("Destruction - Move assigmnent")
    {
        // GIVEN
        CommandRecorder commandRecorder = device.createCommandRecorder();
        Handle<CommandBuffer_t> cbHandle;

        // WHEN
        CommandBuffer commandBuffer = commandRecorder.finish();
        cbHandle = commandBuffer.handle();

        // THEN
        CHECK(commandRecorder.isValid());
        CHECK(commandBuffer.isValid());
        CHECK(cbHandle.isValid());
        CHECK(api->resourceManager()->getCommandBuffer(cbHandle) != nullptr);

        // WHEN
        commandBuffer = {};

        // THEN
        CHECK(api->resourceManager()->getCommandBuffer(cbHandle) == nullptr);
    }

    SUBCASE("Compare default contructed Buffers")
    {
        // GIVEN
        CommandBuffer a;
        CommandBuffer b;

        // THEN
        CHECK(a == b);
    }

    SUBCASE("Compare device create buffers")
    {
        // GIVEN
        CommandRecorder commandRecorderA = device.createCommandRecorder();
        CommandRecorder commandRecorderB = device.createCommandRecorder();

        // WHEN
        CommandBuffer a = commandRecorderA.finish();
        CommandBuffer b = commandRecorderB.finish();

        // THEN
        CHECK(a != b);
    }
}

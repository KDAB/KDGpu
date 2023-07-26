/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/command_buffer.h>
#include <KDGpu/command_recorder.h>
#include <KDGpu/device.h>
#include <KDGpu/instance.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/buffer.h>
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

TEST_CASE("CommandBuffer")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "CommandBuffer",
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

    SUBCASE("Destruction - Move assignment")
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

    SUBCASE("Compare default constructed Buffers")
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

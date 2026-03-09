/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2026 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/config.h>
#include <KDGpu/timeline_semaphore.h>
#include <KDGpu/device.h>
#include <KDGpu/instance.h>
#include <KDGpu/queue.h>
#include <KDGpu/command_recorder.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDGpu;

TEST_SUITE("TimelineSemaphore")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "TimelineSemaphore",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);
    Device device = discreteGPUAdapter->createDevice(DeviceOptions{
            .requestedFeatures = discreteGPUAdapter->features(),
    });
    const bool supportsTimelineSemaphores = discreteGPUAdapter->features().timelineSemaphore;

    TEST_CASE("Construction" * doctest::skip(!supportsTimelineSemaphores))
    {
        SUBCASE("A default constructed TimelineSemaphore is invalid")
        {
            // GIVEN
            TimelineSemaphore s;
            // THEN
            REQUIRE(!s.isValid());
        }

        SUBCASE("A constructed TimelineSemaphore from a Vulkan API is valid")
        {
            // GIVEN
            const TimelineSemaphoreOptions options{};

            // WHEN
            TimelineSemaphore s = device.createTimelineSemaphore(options);

            // THEN
            CHECK(s.isValid());
        }

        SUBCASE("Initial value is respected")
        {
            // GIVEN
            const TimelineSemaphoreOptions options{ .initialValue = 42 };

            // WHEN
            TimelineSemaphore s = device.createTimelineSemaphore(options);

            // THEN
            REQUIRE(s.isValid());
            CHECK(s.value() == 42);
        }

        SUBCASE("Default initial value is zero")
        {
            // GIVEN / WHEN
            TimelineSemaphore s = device.createTimelineSemaphore();

            // THEN
            REQUIRE(s.isValid());
            CHECK(s.value() == 0);
        }

        SUBCASE("Move constructor")
        {
            // GIVEN
            TimelineSemaphore s1 = device.createTimelineSemaphore();
            REQUIRE(s1.isValid());

            // WHEN
            TimelineSemaphore s2(std::move(s1));

            // THEN
            CHECK(!s1.isValid());
            CHECK(s2.isValid());
        }

        SUBCASE("Move assignment")
        {
            // GIVEN
            TimelineSemaphore s1 = device.createTimelineSemaphore();
            TimelineSemaphore s2 = device.createTimelineSemaphore();
            REQUIRE(s1.isValid());
            REQUIRE(s2.isValid());
            const auto s1Handle = s1.handle();

            // WHEN
            s2 = std::move(s1);

            // THEN
            CHECK(!s1.isValid());
            CHECK(s2.isValid());
            CHECK(s2.handle() == s1Handle);
        }
    }

    TEST_CASE("Destruction" * doctest::skip(!supportsTimelineSemaphores))
    {
        Handle<TimelineSemaphore_t> semaphoreHandle;

        SUBCASE("Going out of scope releases resource")
        {
            {
                // WHEN
                TimelineSemaphore s = device.createTimelineSemaphore();
                semaphoreHandle = s.handle();

                // THEN
                CHECK(s.isValid());
                CHECK(semaphoreHandle.isValid());
                CHECK(api->resourceManager()->getTimelineSemaphore(semaphoreHandle) != nullptr);
            }

            // THEN - resource is gone after destructor
            CHECK(api->resourceManager()->getTimelineSemaphore(semaphoreHandle) == nullptr);
        }

        SUBCASE("Move-assign to empty releases resource")
        {
            // WHEN
            TimelineSemaphore s = device.createTimelineSemaphore();
            semaphoreHandle = s.handle();

            CHECK(api->resourceManager()->getTimelineSemaphore(semaphoreHandle) != nullptr);

            // WHEN
            s = {};

            // THEN
            CHECK(api->resourceManager()->getTimelineSemaphore(semaphoreHandle) == nullptr);
        }
    }

    TEST_CASE("CPU Signal and Query" * doctest::skip(!supportsTimelineSemaphores))
    {
        SUBCASE("signal() advances value()")
        {
            // GIVEN
            TimelineSemaphore s = device.createTimelineSemaphore({ .initialValue = 0 });
            REQUIRE(s.isValid());
            REQUIRE(s.value() == 0);

            // WHEN
            s.signal(5);

            // THEN
            CHECK(s.value() == 5);
        }

        SUBCASE("Multiple signal() calls advance value monotonically")
        {
            // GIVEN
            TimelineSemaphore s = device.createTimelineSemaphore({ .initialValue = 0 });
            REQUIRE(s.isValid());

            // WHEN
            s.signal(1);
            // THEN
            CHECK(s.value() == 1);

            // WHEN
            s.signal(10);
            // THEN
            CHECK(s.value() == 10);

            // WHEN
            s.signal(100);
            // THEN
            CHECK(s.value() == 100);
        }
    }

    TEST_CASE("CPU Wait" * doctest::skip(!supportsTimelineSemaphores))
    {
        SUBCASE("wait() on already-reached value returns Success immediately")
        {
            // GIVEN
            TimelineSemaphore s = device.createTimelineSemaphore({ .initialValue = 10 });
            REQUIRE(s.isValid());

            // WHEN
            const TimelineSemaphoreWaitResult result = s.wait(10);

            // THEN
            CHECK(result == TimelineSemaphoreWaitResult::Success);
        }

        SUBCASE("wait() on a value already exceeded returns Success")
        {
            // GIVEN
            TimelineSemaphore s = device.createTimelineSemaphore({ .initialValue = 0 });
            REQUIRE(s.isValid());
            s.signal(5);

            // WHEN
            const TimelineSemaphoreWaitResult result = s.wait(3);

            // THEN
            CHECK(result == TimelineSemaphoreWaitResult::Success);
        }
    }

    TEST_CASE("GPU submit with timeline semaphore" * doctest::skip(!supportsTimelineSemaphores))
    {
        SUBCASE("GPU signals timeline semaphore, CPU waits on it")
        {
            // GIVEN
            TimelineSemaphore sem = device.createTimelineSemaphore({ .initialValue = 0 });
            REQUIRE(sem.isValid());

            CommandRecorder recorder = device.createCommandRecorder();
            REQUIRE(recorder.isValid());
            CommandBuffer commandBuffer = recorder.finish();

            // WHEN - submit empty command buffer that signals the timeline semaphore at value 1
            device.queues().front().submit(SubmitOptions{
                    .commandBuffers = { commandBuffer },
                    .signalTimelineSemaphores = {
                            TimelineSemaphoreSubmitSignalInfo{
                                    .semaphore = sem.handle(),
                                    .value = 1,
                            },
                    },
            });

            // THEN - CPU wait should succeed once GPU signals
            const TimelineSemaphoreWaitResult result = sem.wait(1);
            CHECK(result == TimelineSemaphoreWaitResult::Success);
            CHECK(sem.value() == 1);
        }

        SUBCASE("Two chained GPU submissions via timeline semaphore")
        {
            // GIVEN
            TimelineSemaphore sem = device.createTimelineSemaphore({ .initialValue = 0 });
            REQUIRE(sem.isValid());

            CommandRecorder recorder1 = device.createCommandRecorder();
            CommandBuffer cb1 = recorder1.finish();

            CommandRecorder recorder2 = device.createCommandRecorder();
            CommandBuffer cb2 = recorder2.finish();

            Queue &queue = device.queues().front();

            // WHEN - first submit signals value 1
            queue.submit(SubmitOptions{
                    .commandBuffers = { cb1 },
                    .signalTimelineSemaphores = {
                            TimelineSemaphoreSubmitSignalInfo{
                                    .semaphore = sem.handle(),
                                    .value = 1,
                            },
                    },
            });

            // WHEN - second submit waits on value 1 and signals value 2
            queue.submit(SubmitOptions{
                    .commandBuffers = { cb2 },
                    .waitTimelineSemaphores = {
                            TimelineSemaphoreSubmitWaitInfo{
                                    .semaphore = sem,
                                    .value = 1,
                            },
                    },
                    .signalTimelineSemaphores = {
                            TimelineSemaphoreSubmitSignalInfo{
                                    .semaphore = sem,
                                    .value = 2,
                            },
                    },
            });

            // THEN - CPU waits for the whole chain to complete
            const TimelineSemaphoreWaitResult result = sem.wait(2);
            CHECK(result == TimelineSemaphoreWaitResult::Success);
            CHECK(sem.value() == 2);
        }
    }
}

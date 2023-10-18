/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/config.h>
#include <KDGpu/fence.h>
#include <KDGpu/device.h>
#include <KDGpu/instance.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDGpu;

TEST_SUITE("Fence")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "Fence",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);
    Device device = discreteGPUAdapter->createDevice();

    TEST_CASE("Construction")
    {
        SUBCASE("A default constructed Fence is invalid")
        {
            // GIVEN
            Fence s;
            // THEN
            REQUIRE(!s.isValid());
        }

        SUBCASE("A constructed Fence from a Vulkan API")
        {
            // GIVEN
            const FenceOptions fenceOptions{};

            // WHEN
            Fence s = device.createFence(fenceOptions);

            // THEN
            CHECK(s.isValid());
        }
#if defined(KDGPU_PLATFORM_LINUX)
        SUBCASE("A constructed Fence from a Vulkan API with external FD")
        {
            // GIVEN
            const FenceOptions fenceOptions{
                .externalFenceHandleType = ExternalFenceHandleTypeFlagBits::OpaqueFD,
            };

            // WHEN
            Fence s = device.createFence(fenceOptions);

            // THEN
            CHECK(s.isValid());
            const HandleOrFD externalHandleOrFD = s.externalFenceHandle();
            CHECK(std::get<int>(externalHandleOrFD) > -1);
        }
#elif defined(KDGPU_PLATFORM_WIN32)
        SUBCASE("A constructed Fence from a Vulkan API with external Handle")
        {
            // GIVEN
            const FenceOptions fenceOptions{
                .externalFenceHandleType = ExternalFenceHandleTypeFlagBits::OpaqueWin32,
            };

            // WHEN
            Fence s = device.createFence(fenceOptions);

            // THEN
            CHECK(s.isValid());
            const HandleOrFD externalHandleOrFD = s.externalFenceHandle();
            CHECK(std::get<HANDLE>(externalHandleOrFD) != nullptr);
        }
#endif
    }

    TEST_CASE("Destruction")
    {
        // GIVEN
        const FenceOptions fenceOptions{};

        Handle<Fence_t> fenceHandle;

        SUBCASE("Going Out Of Scope")
        {
            {
                // WHEN
                Fence s = device.createFence(fenceOptions);
                fenceHandle = s.handle();

                // THEN
                CHECK(s.isValid());
                CHECK(fenceHandle.isValid());
                CHECK(api->resourceManager()->getFence(fenceHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getFence(fenceHandle) == nullptr);
        }

        SUBCASE("Move assignment")
        {
            // WHEN
            Fence s = device.createFence(fenceOptions);
            fenceHandle = s.handle();

            // THEN
            CHECK(s.isValid());
            CHECK(fenceHandle.isValid());
            CHECK(api->resourceManager()->getFence(fenceHandle) != nullptr);

            // WHEN
            s = {};

            // THEN
            CHECK(api->resourceManager()->getFence(fenceHandle) == nullptr);
        }
    }

    TEST_CASE("Comparison")
    {
        SUBCASE("Compare default constructed Fences")
        {
            // GIVEN
            Fence a;
            Fence b;

            // THEN
            CHECK(a == b);
        }

        SUBCASE("Compare device created Fences")
        {
            // GIVEN
            const FenceOptions fenceOptions{};

            // WHEN
            Fence a = device.createFence(fenceOptions);
            Fence b = device.createFence(fenceOptions);

            // THEN
            CHECK(a != b);
        }
    }
}

/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/config.h>
#include <KDGpu/gpu_semaphore.h>
#include <KDGpu/device.h>
#include <KDGpu/instance.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDGpu;

TEST_SUITE("GPU_Semaphore")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "GPU_Semaphore",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);
    Device device = discreteGPUAdapter->createDevice();

    TEST_CASE("Construction")
    {
        SUBCASE("A default constructed GpuSemaphore is invalid")
        {
            // GIVEN
            GpuSemaphore s;
            // THEN
            REQUIRE(!s.isValid());
        }

        SUBCASE("A constructed GpuSemaphore from a Vulkan API")
        {
            // GIVEN
            const GpuSemaphoreOptions gpuSemaphoreOptions{};

            // WHEN
            GpuSemaphore s = device.createGpuSemaphore(gpuSemaphoreOptions);

            // THEN
            CHECK(s.isValid());
        }

#if defined(KDGPU_PLATFORM_LINUX)
        SUBCASE("A constructed GpuSemaphore from a Vulkan API with external FD")
        {
            // GIVEN
            const GpuSemaphoreOptions gpuSemaphoreOptions{
                .externalSemaphoreHandleType = ExternalSemaphoreHandleTypeFlagBits::OpaqueFD,
            };

            // WHEN
            GpuSemaphore s = device.createGpuSemaphore(gpuSemaphoreOptions);

            // THEN
            CHECK(s.isValid());
            const HandleOrFD externalHandleOrFD = s.externalSemaphoreHandle();
            CHECK(std::get<int>(externalHandleOrFD) > -1);
        }
#elif defined(KDGPU_PLATFORM_WIN32)
        SUBCASE("A constructed GpuSemaphore from a Vulkan API with external Handle")
        {
            // GIVEN
            const GpuSemaphoreOptions gpuSemaphoreOptions{
                .externalSemaphoreHandleType = ExternalSemaphoreHandleTypeFlagBits::OpaqueWin32,
            };

            // WHEN
            GpuSemaphore s = device.createGpuSemaphore(gpuSemaphoreOptions);

            // THEN
            CHECK(s.isValid());
            const HandleOrFD externalHandleOrFD = s.externalSemaphoreHandle();
            CHECK(std::get<HANDLE>(externalHandleOrFD) != nullptr);
        }
#endif
    }

    TEST_CASE("Destruction")
    {
        // GIVEN
        const GpuSemaphoreOptions gpuSemaphoreOptions{};

        Handle<GpuSemaphore_t> gpuSemaphoreHandle;

        SUBCASE("Going Out Of Scope")
        {
            {
                // WHEN
                GpuSemaphore s = device.createGpuSemaphore(gpuSemaphoreOptions);
                gpuSemaphoreHandle = s.handle();

                // THEN
                CHECK(s.isValid());
                CHECK(gpuSemaphoreHandle.isValid());
                CHECK(api->resourceManager()->getGpuSemaphore(gpuSemaphoreHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getGpuSemaphore(gpuSemaphoreHandle) == nullptr);
        }

        SUBCASE("Move assignment")
        {
            // WHEN
            GpuSemaphore s = device.createGpuSemaphore(gpuSemaphoreOptions);
            gpuSemaphoreHandle = s.handle();

            // THEN
            CHECK(s.isValid());
            CHECK(gpuSemaphoreHandle.isValid());
            CHECK(api->resourceManager()->getGpuSemaphore(gpuSemaphoreHandle) != nullptr);

            // WHEN
            s = {};

            // THEN
            CHECK(api->resourceManager()->getGpuSemaphore(gpuSemaphoreHandle) == nullptr);
        }
    }
}

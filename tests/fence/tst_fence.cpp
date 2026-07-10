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

#if defined(KDGPU_PLATFORM_LINUX)
    const ExternalFenceProperties externalFenceProperties = discreteGPUAdapter->externalFenceProperties(ExternalFenceHandleTypeFlagBits::OpaqueFD);
#elif defined(KDGPU_PLATFORM_WIN32)
    const ExternalFenceProperties externalFenceProperties = discreteGPUAdapter->externalFenceProperties(ExternalFenceHandleTypeFlagBits::OpaqueWin32);
#else
    const ExternalFenceProperties externalFenceProperties;
#endif
    const bool supportsFenceExport = externalFenceProperties.externalFenceFeatures.testFlag(KDGpu::ExternalFenceFeatureFlagBits::Exportable);
    const bool supportsFenceImport = externalFenceProperties.externalFenceFeatures.testFlag(KDGpu::ExternalFenceFeatureFlagBits::Importable);

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

        SUBCASE("Move constructor & move assignment")
        {
            // GIVEN
            const FenceOptions fenceOptions{};

            // WHEN
            Fence s1 = device.createFence(fenceOptions);
            REQUIRE(s1.isValid());

            Fence s2(std::move(s1));

            // THEN
            CHECK(!s1.isValid());
            CHECK(s2.isValid());

            // WHEN
            Fence s3 = device.createFence(fenceOptions);
            const auto s2Handle = s2.handle();
            s3 = std::move(s2);

            // THEN
            CHECK(s3.isValid());
            CHECK(!s2.isValid());
            CHECK(s3.handle() == s2Handle);
        }
    }

    TEST_CASE("Export" * doctest::skip(!supportsFenceExport))
    {
#if defined(KDGPU_PLATFORM_LINUX)
        SUBCASE("A constructed Fence from a Vulkan API with external FD")
        {
            // GIVEN
            const ExternalFenceProperties fenceProperties = discreteGPUAdapter->externalFenceProperties(ExternalFenceHandleTypeFlagBits::OpaqueFD);

            // THEN
            REQUIRE(fenceProperties.externalFenceFeatures.testFlag(KDGpu::ExternalFenceFeatureFlagBits::Exportable));

            // WHEN
            const FenceOptions fenceOptions{
                .externalFenceHandleType = ExternalFenceHandleTypeFlagBits::OpaqueFD,
            };

            const Fence s = device.createFence(fenceOptions);

            // THEN
            CHECK(s.isValid());
            const HandleOrFD externalHandleOrFD = s.externalFenceHandle();
            CHECK(std::get<int>(externalHandleOrFD) > -1);
        }
#elif defined(KDGPU_PLATFORM_WIN32)
        SUBCASE("A constructed Fence from a Vulkan API with external Handle")
        {
            // GIVEN
            const ExternalFenceProperties fenceProperties = discreteGPUAdapter->externalFenceProperties(ExternalFenceHandleTypeFlagBits::OpaqueWin32);

            // THEN
            REQUIRE(fenceProperties.externalFenceFeatures.testFlag(KDGpu::ExternalFenceFeatureFlagBits::Exportable));

            // WHEN
            const FenceOptions fenceOptions{
                .externalFenceHandleType = ExternalFenceHandleTypeFlagBits::OpaqueWin32,
            };

            const Fence s = device.createFence(fenceOptions);

            // THEN
            CHECK(s.isValid());
            const HandleOrFD externalHandleOrFD = s.externalFenceHandle();
            CHECK(std::get<HANDLE>(externalHandleOrFD) != nullptr);
        }
#endif
    }

    TEST_CASE("Import" * doctest::skip(!supportsFenceImport))
    {
#if defined(KDGPU_PLATFORM_LINUX)
        SUBCASE("A constructed Fence from a Vulkan API from an imported external FD")
        {
            // GIVEN
            const ExternalFenceProperties fenceProperties = discreteGPUAdapter->externalFenceProperties(ExternalFenceHandleTypeFlagBits::OpaqueFD);

            // THEN
            REQUIRE(fenceProperties.externalFenceFeatures.testFlag(KDGpu::ExternalFenceFeatureFlagBits::Exportable));
            REQUIRE(fenceProperties.externalFenceFeatures.testFlag(KDGpu::ExternalFenceFeatureFlagBits::Importable));

            {
                // WHEN
                const FenceOptions fenceOptions{
                    .externalFenceHandleType = ExternalFenceHandleTypeFlagBits::OpaqueFD,
                };

                const Fence s = device.createFence(fenceOptions);

                // THEN
                CHECK(s.isValid());
                const HandleOrFD externalHandleOrFD = s.externalFenceHandle();
                CHECK(std::get<int>(externalHandleOrFD) > -1);

                // WHEN
                const FenceOptions importedFenceOptions{
                    .importFenceHandle = externalHandleOrFD,
                    .importFenceHandleType = ExternalFenceHandleTypeFlagBits::OpaqueFD,
                    .importTransference = ImportFenceTransferenceTypeFlagsBits::Temporary,
                };

                const Fence importedFence = device.createFence(importedFenceOptions);

                // THEN
                CHECK(importedFence.isValid());
            }
            {
                // WHEN
                const FenceOptions fenceOptions{
                    .externalFenceHandleType = ExternalFenceHandleTypeFlagBits::OpaqueFD,
                };

                const Fence s = device.createFence(fenceOptions);

                // THEN
                CHECK(s.isValid());
                const HandleOrFD externalHandleOrFD = s.externalFenceHandle();
                CHECK(std::get<int>(externalHandleOrFD) > -1);

                // WHEN
                const FenceOptions importedFenceOptions{
                    .importFenceHandle = externalHandleOrFD,
                    .importFenceHandleType = ExternalFenceHandleTypeFlagBits::OpaqueFD,
                    .importTransference = ImportFenceTransferenceTypeFlagsBits::Permanent,
                };

                const Fence importedFence = device.createFence(importedFenceOptions);

                // THEN
                CHECK(importedFence.isValid());
            }
        }
#elif defined(KDGPU_PLATFORM_WIN32)
        SUBCASE("A constructed Fence from a Vulkan API from an imported external Handle")
        {
            // GIVEN
            const ExternalFenceProperties fenceProperties = discreteGPUAdapter->externalFenceProperties(ExternalFenceHandleTypeFlagBits::OpaqueWin32);

            // THEN
            REQUIRE(fenceProperties.externalFenceFeatures.testFlag(KDGpu::ExternalFenceFeatureFlagBits::Exportable));
            REQUIRE(fenceProperties.externalFenceFeatures.testFlag(KDGpu::ExternalFenceFeatureFlagBits::Importable));

            {
                // WHEN
                const FenceOptions fenceOptions{
                    .externalFenceHandleType = ExternalFenceHandleTypeFlagBits::OpaqueWin32,
                };

                const Fence s = device.createFence(fenceOptions);

                // THEN
                CHECK(s.isValid());
                const HandleOrFD externalHandleOrFD = s.externalFenceHandle();
                CHECK(std::get<HANDLE>(externalHandleOrFD) != nullptr);

                // WHEN
                const FenceOptions importedFenceOptions{
                    .importFenceHandle = externalHandleOrFD,
                    .importFenceHandleType = ExternalFenceHandleTypeFlagBits::OpaqueWin32,
                    .importTransference = ImportFenceTransferenceTypeFlagsBits::Temporary,
                };

                const Fence importedFence = device.createFence(importedFenceOptions);

                // THEN
                CHECK(importedFence.isValid());
            }
            {
                // WHEN
                const FenceOptions fenceOptions{
                    .externalFenceHandleType = ExternalFenceHandleTypeFlagBits::OpaqueWin32,
                };

                const Fence s = device.createFence(fenceOptions);

                // THEN
                CHECK(s.isValid());
                const HandleOrFD externalHandleOrFD = s.externalFenceHandle();
                CHECK(std::get<HANDLE>(externalHandleOrFD) != nullptr);

                // WHEN
                const FenceOptions importedFenceOptions{
                    .importFenceHandle = externalHandleOrFD,
                    .importFenceHandleType = ExternalFenceHandleTypeFlagBits::OpaqueWin32,
                    .importTransference = ImportFenceTransferenceTypeFlagsBits::Permanent,
                };

                const Fence importedFence = device.createFence(importedFenceOptions);

                // THEN
                CHECK(importedFence.isValid());
            }
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

    TEST_CASE("Reset")
    {
        // GIVEN
        const FenceOptions fenceOptions{};

        // WHEN
        Fence a = device.createFence(fenceOptions);

        // THEN
        REQUIRE(a.isValid());
        CHECK(a.status() == FenceStatus::Signalled);

        // WHEN
        a.reset();

        // THEN
        CHECK(a.status() == FenceStatus::Unsignalled);
    }

    TEST_CASE("Wait")
    {
        // GIVEN
        const FenceOptions fenceOptions{
            .createSignalled = false,
        };

        // WHEN
        Fence a = device.createFence(fenceOptions);

        // THEN
        REQUIRE(a.isValid());
        CHECK(a.status() == FenceStatus::Unsignalled);

        // GIVEN
        CommandRecorder c = device.createCommandRecorder();

        // THEN
        CHECK(c.isValid());

        // WHEN
        CommandBuffer commandBuffer = c.finish();
        device.queues().front().submit(SubmitOptions{
                .commandBuffers = { commandBuffer },
                .signalFence = a,
        });

        a.wait();

        // THEN
        CHECK(a.status() == FenceStatus::Signalled);
    }
}

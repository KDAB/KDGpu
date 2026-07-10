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

#if defined(KDGPU_PLATFORM_LINUX)
    const ExternalSemaphoreProperties externalSemaphoreProperties = discreteGPUAdapter->externalSemaphoreProperties(ExternalSemaphoreHandleTypeFlagBits::OpaqueFD);
#elif defined(KDGPU_PLATFORM_WIN32)
    const ExternalSemaphoreProperties externalSemaphoreProperties = discreteGPUAdapter->externalSemaphoreProperties(ExternalSemaphoreHandleTypeFlagBits::OpaqueWin32);
#else
    const ExternalSemaphoreProperties externalSemaphoreProperties;
#endif
    const bool supportsSemaphoreExport = externalSemaphoreProperties.externalSemaphoreFeatures.testFlag(KDGpu::ExternalSemaphoreFeatureFlagBits::Exportable);
    const bool supportsSemaphoreImport = externalSemaphoreProperties.externalSemaphoreFeatures.testFlag(KDGpu::ExternalSemaphoreFeatureFlagBits::Importable);

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
    }

    TEST_CASE("Export" * doctest::skip(!supportsSemaphoreExport))
    {
#if defined(KDGPU_PLATFORM_LINUX)
        SUBCASE("A constructed GpuSemaphore from a Vulkan API which exposes external FD")
        {
            // GIVEN
            const ExternalSemaphoreProperties semaphoreProperties = discreteGPUAdapter->externalSemaphoreProperties(ExternalSemaphoreHandleTypeFlagBits::OpaqueFD);

            // THEN
            REQUIRE(semaphoreProperties.externalSemaphoreFeatures.testFlag(KDGpu::ExternalSemaphoreFeatureFlagBits::Exportable));

            // WHEN
            const GpuSemaphoreOptions gpuSemaphoreOptions{
                .externalSemaphoreHandleType = ExternalSemaphoreHandleTypeFlagBits::OpaqueFD,
            };

            GpuSemaphore s = device.createGpuSemaphore(gpuSemaphoreOptions);

            // THEN
            CHECK(s.isValid());
            const HandleOrFD externalHandleOrFD = s.externalSemaphoreHandle();
            CHECK(std::get<int>(externalHandleOrFD) > -1);
        }

#elif defined(KDGPU_PLATFORM_WIN32)
        SUBCASE("A constructed GpuSemaphore from a Vulkan API which exposes external Handle")
        {
            // GIVEN
            const ExternalSemaphoreProperties semaphoreProperties = discreteGPUAdapter->externalSemaphoreProperties(ExternalSemaphoreHandleTypeFlagBits::OpaqueWin32);

            // THEN
            REQUIRE(semaphoreProperties.externalSemaphoreFeatures.testFlag(KDGpu::ExternalSemaphoreFeatureFlagBits::Exportable));

            // WHEN
            const GpuSemaphoreOptions gpuSemaphoreOptions{
                .externalSemaphoreHandleType = ExternalSemaphoreHandleTypeFlagBits::OpaqueWin32,
            };

            GpuSemaphore s = device.createGpuSemaphore(gpuSemaphoreOptions);

            // THEN
            CHECK(s.isValid());
            const HandleOrFD externalHandleOrFD = s.externalSemaphoreHandle();
            CHECK(std::get<HANDLE>(externalHandleOrFD) != nullptr);
        }
#endif
    }

    TEST_CASE("Import" * doctest::skip(!supportsSemaphoreImport))
    {
#if defined(KDGPU_PLATFORM_LINUX)
        SUBCASE("A constructed GpuSemaphore from a Vulkan API from an imported external FD")
        {
            // GIVEN
            const ExternalSemaphoreProperties semaphoreProperties = discreteGPUAdapter->externalSemaphoreProperties(ExternalSemaphoreHandleTypeFlagBits::OpaqueFD);

            // THEN
            REQUIRE(semaphoreProperties.externalSemaphoreFeatures.testFlag(KDGpu::ExternalSemaphoreFeatureFlagBits::Exportable));
            REQUIRE(semaphoreProperties.externalSemaphoreFeatures.testFlag(KDGpu::ExternalSemaphoreFeatureFlagBits::Importable));

            {
                // WHEN
                const GpuSemaphoreOptions exportedGpuSemaphoreOptions{
                    .externalSemaphoreHandleType = ExternalSemaphoreHandleTypeFlagBits::OpaqueFD,
                };

                const GpuSemaphore exportedSemaphore = device.createGpuSemaphore(exportedGpuSemaphoreOptions);

                // THEN
                CHECK(exportedSemaphore.isValid());
                const HandleOrFD externalHandleOrFD = exportedSemaphore.externalSemaphoreHandle();
                CHECK(std::get<int>(externalHandleOrFD) > -1);

                // WHEN
                const GpuSemaphoreOptions importedGpuSemaphoreOptions{
                    .importSemaphoreHandle = externalHandleOrFD,
                    .importSemaphoreHandleType = ExternalSemaphoreHandleTypeFlagBits::OpaqueFD,
                    .importTransference = ImportSemaphoreTransferenceTypeFlagsBits::Temporary,
                };

                const GpuSemaphore importedSemaphore = device.createGpuSemaphore(importedGpuSemaphoreOptions);

                // THEN
                CHECK(importedSemaphore.isValid());
            }
            {
                // WHEN
                const GpuSemaphoreOptions exportedGpuSemaphoreOptions{
                    .externalSemaphoreHandleType = ExternalSemaphoreHandleTypeFlagBits::OpaqueFD,
                };

                const GpuSemaphore exportedSemaphore = device.createGpuSemaphore(exportedGpuSemaphoreOptions);

                // THEN
                CHECK(exportedSemaphore.isValid());
                const HandleOrFD externalHandleOrFD = exportedSemaphore.externalSemaphoreHandle();
                CHECK(std::get<int>(externalHandleOrFD) > -1);

                // WHEN
                const GpuSemaphoreOptions importedGpuSemaphoreOptions{
                    .importSemaphoreHandle = externalHandleOrFD,
                    .importSemaphoreHandleType = ExternalSemaphoreHandleTypeFlagBits::OpaqueFD,
                    .importTransference = ImportSemaphoreTransferenceTypeFlagsBits::Permanent,
                };

                const GpuSemaphore importedSemaphore = device.createGpuSemaphore(importedGpuSemaphoreOptions);

                // THEN
                CHECK(importedSemaphore.isValid());
            }
        }
#elif defined(KDGPU_PLATFORM_WIN32)
        SUBCASE("A constructed GpuSemaphore from a Vulkan API from an imported external Handle")
        {
            // GIVEN
            const ExternalSemaphoreProperties semaphoreProperties = discreteGPUAdapter->externalSemaphoreProperties(ExternalSemaphoreHandleTypeFlagBits::OpaqueWin32);

            // THEN
            REQUIRE(semaphoreProperties.externalSemaphoreFeatures.testFlag(KDGpu::ExternalSemaphoreFeatureFlagBits::Exportable));
            REQUIRE(semaphoreProperties.externalSemaphoreFeatures.testFlag(KDGpu::ExternalSemaphoreFeatureFlagBits::Importable));

            {
                // WHEN
                const GpuSemaphoreOptions exportedGpuSemaphoreOptions{
                    .externalSemaphoreHandleType = ExternalSemaphoreHandleTypeFlagBits::OpaqueWin32,
                };

                const GpuSemaphore exportedSemaphore = device.createGpuSemaphore(exportedGpuSemaphoreOptions);

                // THEN
                CHECK(exportedSemaphore.isValid());
                const HandleOrFD externalHandleOrFD = exportedSemaphore.externalSemaphoreHandle();
                CHECK(std::get<HANDLE>(externalHandleOrFD) != nullptr);

                // WHEN
                const GpuSemaphoreOptions importedGpuSemaphoreOptions{
                    .importSemaphoreHandle = externalHandleOrFD,
                    .importSemaphoreHandleType = ExternalSemaphoreHandleTypeFlagBits::OpaqueWin32,
                    .importTransference = ImportSemaphoreTransferenceTypeFlagsBits::Temporary,
                };

                const GpuSemaphore importedSemaphore = device.createGpuSemaphore(importedGpuSemaphoreOptions);

                // THEN
                CHECK(importedSemaphore.isValid());
            }
            {
                // WHEN
                const GpuSemaphoreOptions exportedGpuSemaphoreOptions{
                    .externalSemaphoreHandleType = ExternalSemaphoreHandleTypeFlagBits::OpaqueWin32,
                };

                const GpuSemaphore exportedSemaphore = device.createGpuSemaphore(exportedGpuSemaphoreOptions);

                // THEN
                CHECK(exportedSemaphore.isValid());
                const HandleOrFD externalHandleOrFD = exportedSemaphore.externalSemaphoreHandle();
                CHECK(std::get<HANDLE>(externalHandleOrFD) != nullptr);

                // WHEN
                const GpuSemaphoreOptions importedGpuSemaphoreOptions{
                    .importSemaphoreHandle = externalHandleOrFD,
                    .importSemaphoreHandleType = ExternalSemaphoreHandleTypeFlagBits::OpaqueWin32,
                    .importTransference = ImportSemaphoreTransferenceTypeFlagsBits::Permanent,
                };

                const GpuSemaphore importedSemaphore = device.createGpuSemaphore(importedGpuSemaphoreOptions);

                // THEN
                CHECK(importedSemaphore.isValid());
            }
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

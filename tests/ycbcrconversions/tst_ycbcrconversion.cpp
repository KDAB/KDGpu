/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/ycbcr_conversion.h>
#include <KDGpu/ycbcr_conversion_options.h>
#include <KDGpu/device.h>
#include <KDGpu/instance.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDGpu;

TEST_SUITE("YCbCrConversions")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "sampler",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);

    TEST_CASE("Construction" * doctest::skip(!discreteGPUAdapter->features().samplerYCbCrConversion))
    {
        Device device = discreteGPUAdapter->createDevice(DeviceOptions{
                .requestedFeatures = {
                        .samplerYCbCrConversion = true,
                },
        });

        SUBCASE("A default constructed YCbCrConversion is invalid")
        {
            // GIVEN
            YCbCrConversion s;
            // THEN
            REQUIRE(!s.isValid());
        }

        SUBCASE("A constructed YCbCrConversion from a Vulkan API")
        {
            // GIVEN
            const YCbCrConversionOptions ycbcrOptions{
                .format = Format::G8_B8_R8_3PLANE_420_UNORM,
            };

            // WHEN
            YCbCrConversion s = device.createYCbCrConversion(ycbcrOptions);

            // THEN
            CHECK(s.isValid());
        }
    }

    TEST_CASE("Destruction" * doctest::skip(!discreteGPUAdapter->features().samplerYCbCrConversion))
    {
        Device device = discreteGPUAdapter->createDevice(DeviceOptions{
                .requestedFeatures = {
                        .samplerYCbCrConversion = true,
                },
        });

        // GIVEN
        const YCbCrConversionOptions ycbcrOptions{
            .format = Format::G8_B8_R8_3PLANE_420_UNORM,
        };

        Handle<YCbCrConversion_t> conversionHandle;

        SUBCASE("Going Out Of Scope")
        {
            {
                // WHEN
                YCbCrConversion s = device.createYCbCrConversion(ycbcrOptions);
                conversionHandle = s.handle();

                // THEN
                CHECK(s.isValid());
                CHECK(conversionHandle.isValid());
                CHECK(api->resourceManager()->getYCbCrConversion(conversionHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getYCbCrConversion(conversionHandle) == nullptr);
        }

        SUBCASE("Move assignment")
        {
            // WHEN
            YCbCrConversion s = device.createYCbCrConversion(ycbcrOptions);
            conversionHandle = s.handle();

            // THEN
            CHECK(s.isValid());
            CHECK(conversionHandle.isValid());
            CHECK(api->resourceManager()->getYCbCrConversion(conversionHandle) != nullptr);

            // WHEN
            s = {};

            // THEN
            CHECK(api->resourceManager()->getYCbCrConversion(conversionHandle) == nullptr);
        }
    }

    TEST_CASE("Comparison" * doctest::skip(!discreteGPUAdapter->features().samplerYCbCrConversion))
    {
        Device device = discreteGPUAdapter->createDevice(DeviceOptions{
                .requestedFeatures = {
                        .samplerYCbCrConversion = true,
                },
        });

        SUBCASE("Compare default constructed YCbCrConversion")
        {
            // GIVEN
            YCbCrConversion a;
            YCbCrConversion b;

            // THEN
            CHECK(a == b);
        }

        SUBCASE("Compare device created YCbCrConversion")
        {
            // GIVEN
            const YCbCrConversionOptions ycbcrOptions{
                .format = Format::G8_B8_R8_3PLANE_420_UNORM,
            };

            // WHEN
            YCbCrConversion a = device.createYCbCrConversion(ycbcrOptions);
            YCbCrConversion b = device.createYCbCrConversion(ycbcrOptions);

            // THEN
            CHECK(a != b);
        }
    }
}

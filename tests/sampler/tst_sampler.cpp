/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/sampler.h>
#include <KDGpu/sampler_options.h>
#include <KDGpu/device.h>
#include <KDGpu/instance.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDGpu;

TEST_SUITE("Sampler")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "sampler",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);

    TEST_CASE("Construction")
    {
        Device device = discreteGPUAdapter->createDevice();

        SUBCASE("A default constructed Sampler is invalid")
        {
            // GIVEN
            Sampler s;
            // THEN
            REQUIRE(!s.isValid());
        }

        SUBCASE("A constructed Sampler from a Vulkan API")
        {
            // GIVEN
            const SamplerOptions samplerOptions{};

            // WHEN
            Sampler s = device.createSampler(samplerOptions);

            // THEN
            CHECK(s.isValid());
        }
    }

    TEST_CASE("Destruction")
    {
        Device device = discreteGPUAdapter->createDevice();

        // GIVEN
        const SamplerOptions samplerOptions{};

        Handle<Sampler_t> samplerHandle;

        SUBCASE("Going Out Of Scope")
        {
            {
                // WHEN
                Sampler s = device.createSampler(samplerOptions);
                samplerHandle = s.handle();

                // THEN
                CHECK(s.isValid());
                CHECK(samplerHandle.isValid());
                CHECK(api->resourceManager()->getSampler(samplerHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getSampler(samplerHandle) == nullptr);
        }

        SUBCASE("Move assignment")
        {
            // WHEN
            Sampler s = device.createSampler(samplerOptions);
            samplerHandle = s.handle();

            // THEN
            CHECK(s.isValid());
            CHECK(samplerHandle.isValid());
            CHECK(api->resourceManager()->getSampler(samplerHandle) != nullptr);

            // WHEN
            s = {};

            // THEN
            CHECK(api->resourceManager()->getSampler(samplerHandle) == nullptr);
        }
    }

    TEST_CASE("Comparison")
    {
        Device device = discreteGPUAdapter->createDevice();

        SUBCASE("Compare default constructed Samplers")
        {
            // GIVEN
            Sampler a;
            Sampler b;

            // THEN
            CHECK(a == b);
        }

        SUBCASE("Compare device created Samplers")
        {
            // GIVEN
            const SamplerOptions samplerOptions{};

            // WHEN
            Sampler a = device.createSampler(samplerOptions);
            Sampler b = device.createSampler(samplerOptions);

            // THEN
            CHECK(a != b);
        }
    }

#if defined(VK_KHR_sampler_ycbcr_conversion)
    TEST_CASE("YUV Sampling" * doctest::skip(!discreteGPUAdapter->features().samplerYCbCrConversion))
    {
        REQUIRE(discreteGPUAdapter->features().samplerYCbCrConversion);

        Device device = discreteGPUAdapter->createDevice(DeviceOptions{
                .requestedFeatures = {
                        .samplerYCbCrConversion = true,
                },
        });

        // GIVEN
        const YCbCrConversion ycbCrConversion = device.createYCbCrConversion(YCbCrConversionOptions{
                .format = Format::G8_B8_R8_3PLANE_420_UNORM,
        });

        const SamplerOptions samplerOptions{
            .u = AddressMode::ClampToEdge,
            .v = AddressMode::ClampToEdge,
            .w = AddressMode::ClampToEdge,
            .yCbCrConversion = ycbCrConversion,
        };

        // THEN
        CHECK(ycbCrConversion.isValid());

        // WHEN
        Sampler s = device.createSampler(samplerOptions);

        // THEN
        CHECK(s.isValid()); // And no validation errors
    }
#endif
}

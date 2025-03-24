/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/texture.h>
#include <KDGpu/texture_options.h>
#include <KDGpu/device.h>
#include <KDGpu/instance.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

#include <set>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDGpu;

TEST_SUITE("TextureView")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "TextureView",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);

    TEST_CASE("Construction")
    {
        Device device = discreteGPUAdapter->createDevice();

        SUBCASE("A default constructed TextureView is invalid")
        {
            // GIVEN
            TextureView tv;
            // THEN
            REQUIRE(!tv.isValid());
        }

        SUBCASE("A constructed Texture from a Vulkan API")
        {
            // GIVEN
            const TextureOptions textureOptions = {
                .type = TextureType::TextureType2D,
                .format = Format::R8G8B8A8_SNORM,
                .extent = { 512, 512, 1 },
                .mipLevels = 1,
                .usage = TextureUsageFlagBits::SampledBit | TextureUsageFlagBits::ColorAttachmentBit,
                .memoryUsage = MemoryUsage::GpuOnly
            };

            // WHEN
            Texture t = device.createTexture(textureOptions);

            // THEN
            CHECK(t.isValid());

            // WHEN
            TextureView tv = t.createView();

            // THEN
            CHECK(tv.isValid());
        }
    }

    TEST_CASE("Destruction")
    {
        Device device = discreteGPUAdapter->createDevice();

        // GIVEN
        const TextureOptions textureOptions = {
            .type = TextureType::TextureType2D,
            .format = Format::R8G8B8A8_SNORM,
            .extent = { 512, 512, 1 },
            .mipLevels = 1,
            .usage = TextureUsageFlagBits::SampledBit,
            .memoryUsage = MemoryUsage::GpuOnly
        };

        Texture t = device.createTexture(textureOptions);

        Handle<TextureView_t> textureViewHandle;

        SUBCASE("Going Out Of Scope")
        {
            {
                // WHEN
                TextureView tv = t.createView();
                textureViewHandle = tv.handle();

                // THEN
                CHECK(t.isValid());
                CHECK(tv.isValid());
                CHECK(textureViewHandle.isValid());
                CHECK(api->resourceManager()->getTextureView(textureViewHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getTextureView(textureViewHandle) == nullptr);
        }

        SUBCASE("Move assignment")
        {
            Device device = discreteGPUAdapter->createDevice();

            // WHEN
            TextureView tv = t.createView();
            textureViewHandle = tv.handle();

            // THEN
            CHECK(t.isValid());
            CHECK(tv.isValid());
            CHECK(textureViewHandle.isValid());
            CHECK(api->resourceManager()->getTextureView(textureViewHandle) != nullptr);

            // WHEN
            tv = {};

            // THEN
            CHECK(api->resourceManager()->getTextureView(textureViewHandle) == nullptr);
        }
    }

    TEST_CASE("Comparison")
    {
        Device device = discreteGPUAdapter->createDevice();

        SUBCASE("Compare default constructed Textures")
        {
            // GIVEN
            TextureView a;
            TextureView b;

            // THEN
            CHECK(a == b);
        }

        SUBCASE("Compare device created Textures")
        {
            // GIVEN
            const TextureOptions textureOptions = {
                .type = TextureType::TextureType2D,
                .format = Format::R8G8B8A8_SNORM,
                .extent = { 512, 512, 1 },
                .mipLevels = 1,
                .usage = TextureUsageFlagBits::SampledBit | TextureUsageFlagBits::ColorAttachmentBit,
                .memoryUsage = MemoryUsage::GpuOnly
            };

            // WHEN
            Texture t = device.createTexture(textureOptions);
            TextureView a = t.createView();
            TextureView b = t.createView();

            // THEN
            CHECK(a != b);
        }
    }

#if defined(VK_KHR_sampler_ycbcr_conversion)
    TEST_CASE("YUV View" * doctest::skip(!discreteGPUAdapter->features().samplerYCbCrConversion))
    {
        REQUIRE(discreteGPUAdapter->features().samplerYCbCrConversion);

        Device device = discreteGPUAdapter->createDevice(DeviceOptions{
                .requestedFeatures = {
                        .samplerYCbCrConversion = true,
                },
        });

        // GIVEN
        Texture t = device.createTexture({
                .type = TextureType::TextureType2D,
                .format = Format::G8_B8_R8_3PLANE_420_UNORM,
                .extent = { 512, 512, 1 },
                .mipLevels = 1,
                .usage = TextureUsageFlagBits::SampledBit,
                .memoryUsage = MemoryUsage::GpuOnly,
        });

        const YCbCrConversion ycbCrConversion = device.createYCbCrConversion(YCbCrConversionOptions{
                .format = Format::G8_B8_R8_3PLANE_420_UNORM,
                .model = SamplerYCbCrModelConversion::YCbCr709, // We want to convert from YCbCr Rec709 to RGB
                .components = ComponentMapping{
                        // Given G8_B8_R8_3PLANE_420, then G = Y, B = Cb, R = Cr
                        // We want to map [Y][Cb][Cr] -> [G][B][R]
                        .r = ComponentSwizzle::R, // Chroma Red -> R
                        .g = ComponentSwizzle::G, // Luma -> G
                        .b = ComponentSwizzle::B, // Chroma Blue -> B
                },
                .xChromaOffset = ChromaLocation::MidPoint,
                .yChromaOffset = ChromaLocation::MidPoint,
                .chromaFilter = FilterMode::Linear,
                .forceExplicitReconstruction = false,
        });

        // THEN
        CHECK(ycbCrConversion.isValid());

        // WHEN
        TextureView a = t.createView(TextureViewOptions{
                .viewType = ViewType::ViewType2D,
                .range = TextureSubresourceRange{
                        .aspectMask = TextureAspectFlagBits::ColorBit,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                },
                .yCbCrConversion = ycbCrConversion,
        });

        // THEN
        CHECK(a.isValid()); // And no validation errors
    }
#endif
}

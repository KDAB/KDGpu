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
    Device device = discreteGPUAdapter->createDevice();

    TEST_CASE("Construction")
    {
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
}

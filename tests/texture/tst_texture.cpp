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

TEST_SUITE("Texture")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "buffer",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);
    Device device = discreteGPUAdapter->createDevice();

    TEST_CASE("Construction")
    {
        SUBCASE("A default constructed Texture is invalid")
        {
            // GIVEN
            Texture t;
            // THEN
            REQUIRE(!t.isValid());
        }

        SUBCASE("A constructed Texture from a Vulkan API")
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

            // WHEN
            Texture t = device.createTexture(textureOptions);

            // THEN
            CHECK(t.isValid());
        }

#if defined(KDGPU_PLATFORM_LINUX)
        SUBCASE("A constructed Texture from a Vulkan API with external FD")
        {
            // GIVEN
            const TextureOptions textureOptions = {
                .type = TextureType::TextureType2D,
                .format = Format::R8G8B8A8_SNORM,
                .extent = { 512, 512, 1 },
                .mipLevels = 1,
                .usage = TextureUsageFlagBits::SampledBit,
                .memoryUsage = MemoryUsage::GpuOnly,
                .externalMemoryHandleType = ExternalMemoryHandleTypeFlagBits::OpaqueFD,
            };

            // WHEN
            Texture t = device.createTexture(textureOptions);

            // THEN
            CHECK(t.isValid());
            const HandleOrFD externalHandleOrFD = t.externalMemoryHandle();
            CHECK(std::get<int>(externalHandleOrFD) > -1);
        }
#elif defined(KDGPU_PLATFORM_WIN32)
        SUBCASE("A constructed Texture from a Vulkan API with external Handle")
        {
            // GIVEN
            const TextureOptions textureOptions = {
                .type = TextureType::TextureType2D,
                .format = Format::R8G8B8A8_SNORM,
                .extent = { 512, 512, 1 },
                .mipLevels = 1,
                .usage = TextureUsageFlagBits::SampledBit,
                .memoryUsage = MemoryUsage::GpuOnly,
                .externalMemoryHandleType = ExternalMemoryHandleTypeFlagBits::OpaqueWin32,
            };

            // WHEN
            Texture t = device.createTexture(textureOptions);

            // THEN
            CHECK(t.isValid());
            const HandleOrFD externalHandleOrFD = t.externalMemoryHandle();
            CHECK(std::get<HANDLE>(externalHandleOrFD) != nullptr);
        }
#endif
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

        Handle<Texture_t> textureHandle;

        SUBCASE("Going Out Of Scope")
        {
            {
                // WHEN
                Texture t = device.createTexture(textureOptions);
                textureHandle = t.handle();

                // THEN
                CHECK(t.isValid());
                CHECK(textureHandle.isValid());
                CHECK(api->resourceManager()->getTexture(textureHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getTexture(textureHandle) == nullptr);
        }

        SUBCASE("Move assignment")
        {
            // WHEN
            Texture t = device.createTexture(textureOptions);
            textureHandle = t.handle();

            // THEN
            CHECK(t.isValid());
            CHECK(textureHandle.isValid());
            CHECK(api->resourceManager()->getTexture(textureHandle) != nullptr);

            // WHEN
            t = {};

            // THEN
            CHECK(api->resourceManager()->getTexture(textureHandle) == nullptr);
        }
    }

    TEST_CASE("Comparison")
    {
        SUBCASE("Compare default constructed Textures")
        {
            // GIVEN
            Texture a;
            Texture b;

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
                .usage = TextureUsageFlagBits::SampledBit,
                .memoryUsage = MemoryUsage::GpuOnly
            };

            // WHEN
            Texture a = device.createTexture(textureOptions);
            Texture b = device.createTexture(textureOptions);

            // THEN
            CHECK(a != b);
        }
    }

    TEST_CASE("MipMap Creation")
    {
        // GIVEN
        const TextureOptions textureOptions = {
            .type = TextureType::TextureType2D,
            .format = Format::R8G8B8A8_SNORM,
            .extent = { 512, 512, 1 },
            .mipLevels = 8,
            .usage = TextureUsageFlagBits::SampledBit | TextureUsageFlagBits::TransferSrcBit | TextureUsageFlagBits::TransferDstBit,
            .memoryUsage = MemoryUsage::GpuOnly
        };

        // WHEN
        Texture t = device.createTexture(textureOptions);

        // THEN
        CHECK(t.isValid());

        // WHEN
        Queue &transferQueue = device.queues().front();
        const bool success = t.generateMipMaps(device, transferQueue, textureOptions, TextureLayout::Undefined);

        // THEN
        CHECK(success);
    }
}

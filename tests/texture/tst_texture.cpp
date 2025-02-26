/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/config.h>
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

    TEST_CASE("Construction")
    {
        Device device = discreteGPUAdapter->createDevice();

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
            const MemoryHandle memoryHandle = t.externalMemoryHandle();
            CHECK(memoryHandle.allocationSize > 0);
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
            const MemoryHandle externalHandleOrFD = t.externalMemoryHandle();
            CHECK(std::get<int>(externalHandleOrFD.handle) > -1);
            CHECK(externalHandleOrFD.allocationSize > 0);
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
            const MemoryHandle externalHandleOrFD = t.externalMemoryHandle();
            CHECK(std::get<HANDLE>(externalHandleOrFD.handle) != nullptr);
            CHECK(externalHandleOrFD.allocationSize > 0);
        }
#endif
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
        Device device = discreteGPUAdapter->createDevice();

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
        Device device = discreteGPUAdapter->createDevice();

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

#if defined(VK_EXT_host_image_copy)
    TEST_CASE("HostCopy")
    {
        REQUIRE(discreteGPUAdapter->features().hostImageCopy);

        Device device = discreteGPUAdapter->createDevice(DeviceOptions{
                .requestedFeatures = {
                        .hostImageCopy = true,
                },
        });

        Queue graphicsQueue;
        const auto &queues = device.queues();
        for (const auto &q : queues) {
            if (q.flags() | QueueFlags(QueueFlagBits::GraphicsBit)) {
                graphicsQueue = q;
                break;
            }
        }

        REQUIRE(device.isValid());
        REQUIRE(graphicsQueue.isValid());

        SUBCASE("Host -> Image")
        {
            // GIVEN
            Texture t = device.createTexture(TextureOptions{
                    .type = TextureType::TextureType2D,
                    .format = Format::R8G8B8A8_SNORM,
                    .extent = { 512, 512, 1 },
                    .mipLevels = 8,
                    .usage = TextureUsageFlagBits::SampledBit | TextureUsageFlagBits::HostTransferBit,
                    .memoryUsage = MemoryUsage::GpuOnly,
                    .initialLayout = TextureLayout::Undefined,
            });

            // THEN
            CHECK(t.isValid());

            // WHEN
            t.hostLayoutTransition(HostLayoutTransition{
                    .oldLayout = TextureLayout::Undefined,
                    .newLayout = TextureLayout::General,
                    .range = {
                            .aspectMask = TextureAspectFlagBits::ColorBit,
                    },
            });

            // THEN -> No Validation error

            std::vector<uint32_t> rawImageData;
            rawImageData.resize(512 * 512);

            // WHEN
            t.copyHostMemoryToTexture(HostMemoryToTextureCopy{
                    .dstTextureLayout = TextureLayout::General,
                    .regions = {
                            HostMemoryToTextureCopyRegion{
                                    .srcHostMemoryPointer = rawImageData.data(),
                                    .srcMemoryRowLength = 0, // Tightly packed
                                    .srcMemoryImageHeight = 0, // Tightly packed,
                                    .dstSubresource = TextureSubresourceLayers{
                                            .aspectMask = TextureAspectFlagBits::ColorBit,
                                            .mipLevel = 0,
                                            .baseArrayLayer = 0,
                                            .layerCount = 1,
                                    },
                                    .dstOffset = { .x = 0, .y = 0, .z = 0 },
                                    .dstExtent = { 512, 512, 1 },
                            },
                    },
                    .flags = { HostImageCopyFlagBits::HostImageMemcpy }, // No Swizzling between host buffer and target texture
            });

            // THEN -> No validation errors
        }

        SUBCASE("Image -> Host")
        {
            // GIVEN
            Texture t = device.createTexture(TextureOptions{
                    .type = TextureType::TextureType2D,
                    .format = Format::R8G8B8A8_SNORM,
                    .extent = { 512, 512, 1 },
                    .mipLevels = 8,
                    .usage = TextureUsageFlagBits::SampledBit | TextureUsageFlagBits::HostTransferBit | TextureUsageFlagBits::TransferDstBit,
                    .memoryUsage = MemoryUsage::GpuOnly,
                    .initialLayout = TextureLayout::Undefined,
            });

            // THEN
            CHECK(t.isValid());

            // WHEN
            t.hostLayoutTransition(HostLayoutTransition{
                    .oldLayout = TextureLayout::Undefined,
                    .newLayout = TextureLayout::General,
                    .range = {
                            .aspectMask = TextureAspectFlagBits::ColorBit,
                    },
            });

            // THEN -> No Validation error

            // WHEN
            CommandRecorder c = device.createCommandRecorder();
            c.clearColorTexture(ClearColorTexture{
                    .texture = t,
                    .layout = TextureLayout::General,
                    .clearValue = ColorClearValue{ .float32 = { 1.0f, 0.0f, 0.0f, 1.0f } },
                    .ranges = {
                            TextureSubresourceRange{
                                    .aspectMask = TextureAspectFlagBits::ColorBit,
                                    .baseMipLevel = 0,
                                    .levelCount = 1,
                            },
                    },
            });

            auto commandBuffer = c.finish();
            graphicsQueue.submit(SubmitOptions{
                    .commandBuffers = { commandBuffer } });
            device.waitUntilIdle();

            // THEN -> No Validation Error

            std::vector<uint32_t> rawImageData;
            rawImageData.resize(512 * 512);

            // WHEN
            t.copyTextureToHostMemory(TextureToHostMemoryCopy{
                    .textureLayout = TextureLayout::General,
                    .regions = {
                            TextureToHostMemoryCopyRegion{
                                    .srcSubresource = TextureSubresourceLayers{
                                            .aspectMask = TextureAspectFlagBits::ColorBit,
                                            .mipLevel = 0,
                                            .baseArrayLayer = 0,
                                            .layerCount = 1,
                                    },
                                    .srcOffset = { .x = 0, .y = 0, .z = 0 },
                                    .srcExtent = { 512, 512, 1 },
                                    .dstHostMemoryPointer = rawImageData.data(),
                                    .dstMemoryRowLength = 0, // Tightly packed
                                    .dstMemoryImageHeight = 0, // Tightly packed,
                            },
                    },
                    .flags = { HostImageCopyFlagBits::HostImageMemcpy }, // No Swizzling between host buffer and target texture
            });

            // THEN -> No validation errors
            std::ranges::all_of(rawImageData.begin(), rawImageData.end(), [](int rgba) {
                return rgba == 0xff0000ff;
            });
        }

        SUBCASE("Image -> Image -> Host")
        {
            // GIVEN
            const TextureOptions textureOptions{
                .type = TextureType::TextureType2D,
                .format = Format::R8G8B8A8_SNORM,
                .extent = { 512, 512, 1 },
                .mipLevels = 8,
                .usage = TextureUsageFlagBits::HostTransferBit | TextureUsageFlagBits::TransferDstBit | TextureUsageFlagBits::TransferSrcBit,
                .memoryUsage = MemoryUsage::GpuOnly,
                .initialLayout = TextureLayout::Undefined,
            };

            // Note: texture -> texture copy requires both textures to be created with exactly the same parameters
            Texture srcT = device.createTexture(textureOptions);
            Texture dstT = device.createTexture(textureOptions);

            // THEN
            CHECK(srcT.isValid());
            CHECK(dstT.isValid());

            // WHEN
            srcT.hostLayoutTransition(HostLayoutTransition{
                    .oldLayout = TextureLayout::Undefined,
                    .newLayout = TextureLayout::General,
                    .range = {
                            .aspectMask = TextureAspectFlagBits::ColorBit,
                    },
            });
            dstT.hostLayoutTransition(HostLayoutTransition{
                    .oldLayout = TextureLayout::Undefined,
                    .newLayout = TextureLayout::General,
                    .range = {
                            .aspectMask = TextureAspectFlagBits::ColorBit,
                    },
            });

            // THEN -> No Validation error

            // WHEN
            CommandRecorder c = device.createCommandRecorder();
            c.clearColorTexture(ClearColorTexture{
                    .texture = srcT,
                    .layout = TextureLayout::General,
                    .clearValue = ColorClearValue{ .float32 = { 1.0f, 0.0f, 0.0f, 1.0f } },
                    .ranges = {
                            TextureSubresourceRange{
                                    .aspectMask = TextureAspectFlagBits::ColorBit,
                                    .baseMipLevel = 0,
                                    .levelCount = 1,
                            },
                    },
            });

            auto commandBuffer = c.finish();
            graphicsQueue.submit(SubmitOptions{
                    .commandBuffers = { commandBuffer } });
            device.waitUntilIdle();

            // THEN -> No Validation Error

            // WHEN

            // host copy srcT -> dstT
            srcT.copyTextureToTextureHost(TextureToTextureCopyHost{
                    .textureLayout = TextureLayout::General,
                    .dstTexture = dstT,
                    .dstTextureLayout = TextureLayout::General,
                    .regions = {
                            TextureToTextureHostCopyRegion{
                                    .srcOffset = { 0, 0, 0 },
                                    .dstOffset = { 0, 0, 0 },
                                    .extent = { 512, 512, 1 },
                            },
                    },
                    .flags = { HostImageCopyFlagBits::HostImageMemcpy }, // No Swizzling between host buffer and target texture
            });

            std::vector<uint32_t> rawImageData;
            rawImageData.resize(512 * 512);

            // copy dstT -> host buffer
            dstT.copyTextureToHostMemory(TextureToHostMemoryCopy{
                    .textureLayout = TextureLayout::General,
                    .regions = {
                            TextureToHostMemoryCopyRegion{
                                    .srcSubresource = TextureSubresourceLayers{
                                            .aspectMask = TextureAspectFlagBits::ColorBit,
                                            .mipLevel = 0,
                                            .baseArrayLayer = 0,
                                            .layerCount = 1,
                                    },
                                    .srcOffset = { .x = 0, .y = 0, .z = 0 },
                                    .srcExtent = { 512, 512, 1 },
                                    .dstHostMemoryPointer = rawImageData.data(),
                                    .dstMemoryRowLength = 0, // Tightly packed
                                    .dstMemoryImageHeight = 0, // Tightly packed,
                            },
                    },
                    .flags = { HostImageCopyFlagBits::HostImageMemcpy }, // No Swizzling between host buffer and target texture
            });

            // THEN -> No validation errors
            std::ranges::all_of(rawImageData.begin(), rawImageData.end(), [](int rgba) {
                return rgba == 0xff0000ff;
            });
        }
    }
#endif
}

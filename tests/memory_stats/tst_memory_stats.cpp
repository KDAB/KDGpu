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

TEST_SUITE("MemoryStats")
{
    std::unique_ptr<VulkanGraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "memory_stats",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);
    Device device = discreteGPUAdapter->createDevice();

    TEST_CASE("Vulkan Memory Stats")
    {
        SUBCASE("Stats with VK_EXT_memory_budget")
        {
            // GIVEN
            const auto extensions = discreteGPUAdapter->extensions();
            const bool supportsMemoryBudget = std::any_of(extensions.cbegin(), extensions.cend(), [](const Extension &extension) {
                return extension.name == VK_EXT_MEMORY_BUDGET_EXTENSION_NAME;
            });

            // THEN
            if (!supportsMemoryBudget) {
                MESSAGE("VK_EXT_memory_budget is not supported by the selected adapter");
                return;
            }
            Device budgetDevice = discreteGPUAdapter->createDevice(DeviceOptions{
                    .extensions = { VK_EXT_MEMORY_BUDGET_EXTENSION_NAME } });
            REQUIRE(budgetDevice.isValid());

            // WHEN
            Texture t1 = budgetDevice.createTexture(TextureOptions{
                    .label = "MemoryBudgetTexture",
                    .type = TextureType::TextureType2D,
                    .format = Format::R8G8B8A8_SNORM,
                    .extent = { 512, 512, 1 },
                    .mipLevels = 1,
                    .usage = TextureUsageFlagBits::SampledBit,
                    .memoryUsage = MemoryUsage::GpuOnly,
            });

            // THEN
            REQUIRE(t1.isValid());
            const auto statsF0 = api->getMemoryStats(budgetDevice);
            CHECK(!statsF0.empty());
            SPDLOG_WARN("Stats Frame 0: {}", statsF0);

            // WHEN
            budgetDevice.setFrameIndex(1);
            Texture t2 = budgetDevice.createTexture(TextureOptions{
                    .label = "MemoryBudgetTexture2",
                    .type = TextureType::TextureType2D,
                    .format = Format::R8G8B8A8_SNORM,
                    .extent = { 512, 512, 1 },
                    .mipLevels = 1,
                    .usage = TextureUsageFlagBits::SampledBit,
                    .memoryUsage = MemoryUsage::GpuOnly,
            });

            // THEN
            REQUIRE(t2.isValid());
            const auto statsF1 = api->getMemoryStats(budgetDevice);
            CHECK(!statsF1.empty());
            SPDLOG_WARN("Stats Frame 1: {}", statsF1);
        }

        SUBCASE("Stats for a Texture")
        {
            // GIVEN
            const TextureOptions textureOptions = {
                .label = "MyTexture",
                .type = TextureType::TextureType2D,
                .format = Format::R8G8B8A8_SNORM,
                .extent = { 512, 512, 1 },
                .mipLevels = 1,
                .usage = TextureUsageFlagBits::SampledBit,
                .memoryUsage = MemoryUsage::GpuOnly,
            };

            // WHEN
            Texture t = device.createTexture(textureOptions);

            // THEN
            CHECK(t.isValid());

            // WHEN
            const std::string stats = api->getMemoryStats(device);

            // THEN
            CHECK(!stats.empty());

            SPDLOG_WARN("Texture stats: {}", stats);
        }

#if defined(KDGPU_PLATFORM_LINUX)
        SUBCASE("Stats for a Texture with external FD")
        {
            // GIVEN
            const TextureOptions textureOptions = {
                .label = "MyTextureExternalFD",
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

            // WHEN
            const std::string stats = api->getMemoryStats(device);

            // THEN
            CHECK(!stats.empty());

            SPDLOG_WARN("Texture with external FD stats: {}", stats);
        }
#elif defined(KDGPU_PLATFORM_WIN32)
        SUBCASE("Stats for a Texture with external Handle")
        {
            // GIVEN
            const TextureOptions textureOptions = {
                .label = "MyTextureExternalHandle",
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

            // WHEN
            const std::string stats = api->getMemoryStats(device);

            // THEN
            CHECK(!stats.empty());

            SPDLOG_WARN("Texture with external Handle stats: {}", stats);
        }
#endif
    }
}

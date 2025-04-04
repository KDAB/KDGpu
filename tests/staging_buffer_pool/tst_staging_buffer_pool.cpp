/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpuUtils/resource_deleter.h>
#include <KDGpuUtils/staging_buffer_pool.h>

#include <KDGpu/device.h>
#include <KDGpu/instance.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

#include <algorithm>
#include <cstring>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

TEST_SUITE("StagingBufferPool")
{
    std::unique_ptr<KDGpu::GraphicsApi> api = std::make_unique<KDGpu::VulkanGraphicsApi>();
    KDGpu::Instance instance = api->createInstance(KDGpu::InstanceOptions{
            .applicationName = "StagingBufferPool",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });
    KDGpu::Adapter *discreteGPUAdapter = instance.selectAdapter(KDGpu::AdapterDeviceType::Default);
    KDGpu::Device device = discreteGPUAdapter->createDevice();
    constexpr size_t MAX_FRAMES_IN_FLIGHT = 2;

    TEST_CASE("Creation and Destruction")
    {
        SUBCASE("can create a staging buffer pool")
        {
            // GIVEN
            KDGpuUtils::ResourceDeleter deleter(&device, MAX_FRAMES_IN_FLIGHT);
            KDGpuUtils::StagingBufferPoolImpl stagingBufferPool(&device, &deleter);

            // THEN
            REQUIRE(stagingBufferPool.bins().empty());
        }

        SUBCASE("staging buffer pool releases resources on destruction")
        {
            // GIVEN
            KDGpuUtils::ResourceDeleter deleter(&device, MAX_FRAMES_IN_FLIGHT);

            // WHEN
            {
                KDGpuUtils::StagingBufferPoolImpl stagingBufferPool(&device, &deleter);
                const std::vector<uint8_t> testData = std::vector<uint8_t>(512, 0xaa);
                stagingBufferPool.stage(testData);
            }

            // THEN
            auto &bins = deleter.frameBins();
            CHECK(bins.size() == 1);
            CHECK(bins[0].resources.get<KDGpu::Buffer>().size() == 1);
        }
    }

    TEST_CASE("Staging buffer")
    {
        SUBCASE("Creates bin if no existing bin is large enough")
        {
            // GIVEN
            KDGpuUtils::ResourceDeleter deleter(&device, MAX_FRAMES_IN_FLIGHT);
            KDGpuUtils::StagingBufferPoolImpl stagingBufferPool(&device, &deleter);

            // WHEN
            const std::vector<uint8_t> testData = std::vector<uint8_t>(512, 0xaa);
            const std::pair<size_t, KDGpu::Handle<KDGpu::Buffer_t>> result = stagingBufferPool.stage(testData);

            // THEN
            REQUIRE(stagingBufferPool.bins().size() == 1);
            const auto &bin = stagingBufferPool.bins()[0];

            CHECK(bin.buffer.isValid());
            CHECK(bin.buffer.handle() == result.second);
            CHECK(bin.isMapped == true);
            CHECK(bin.m_allocations.size() == 1);
            CHECK(bin.m_allocations[0].offset == 0);
            CHECK(bin.m_allocations[0].offset == result.first);
            CHECK(bin.m_allocations[0].size == 512);
            CHECK(std::memcmp(bin.mapped, testData.data(), 512) == 0);
        }

        SUBCASE("Reuses last bin if content fits")
        {
            KDGpuUtils::ResourceDeleter deleter(&device, MAX_FRAMES_IN_FLIGHT);
            KDGpuUtils::StagingBufferPoolImpl<1, 1024> stagingBufferPool(&device, &deleter);

            // WHEN
            const std::vector<uint8_t> testData = std::vector<uint8_t>(512, 0xaa);
            const std::pair<size_t, KDGpu::Handle<KDGpu::Buffer_t>> r1 = stagingBufferPool.stage(testData);
            const std::pair<size_t, KDGpu::Handle<KDGpu::Buffer_t>> r2 = stagingBufferPool.stage(testData);

            // THEN
            REQUIRE(stagingBufferPool.bins().size() == 1);
            const auto &bin = stagingBufferPool.bins()[0];

            CHECK(bin.buffer.isValid());
            CHECK(bin.buffer.handle() == r1.second);
            CHECK(bin.buffer.handle() == r2.second);
            CHECK(bin.isMapped == true);
            CHECK(bin.m_allocations.size() == 2);
            CHECK(bin.m_allocations[0].offset == 0);
            CHECK(bin.m_allocations[0].offset == r1.first);
            CHECK(bin.m_allocations[0].size == 512);
            CHECK(bin.m_allocations[1].offset == 512);
            CHECK(bin.m_allocations[1].offset == r2.first);
            CHECK(bin.m_allocations[1].size == 512);
            CHECK(std::memcmp(bin.mapped, testData.data(), 512) == 0);
            CHECK(std::memcmp(reinterpret_cast<uint8_t *>(bin.mapped) + 512, testData.data(), 512) == 0);
        }

        SUBCASE("Allocates new bin if existing bins not large enough")
        {
            // GIVEN
            KDGpuUtils::ResourceDeleter deleter(&device, MAX_FRAMES_IN_FLIGHT);
            KDGpuUtils::StagingBufferPoolImpl<1, 1024> stagingBufferPool(&device, &deleter);

            // WHEN
            const std::vector<uint8_t> smallTestData = std::vector<uint8_t>(512, 0xaa);
            const std::vector<uint8_t> bigTestData = std::vector<uint8_t>(768, 0xee);
            const std::pair<size_t, KDGpu::Handle<KDGpu::Buffer_t>> r1 = stagingBufferPool.stage(smallTestData);
            const std::pair<size_t, KDGpu::Handle<KDGpu::Buffer_t>> r2 = stagingBufferPool.stage(bigTestData);

            // THEN
            REQUIRE(stagingBufferPool.bins().size() == 2);
            const auto &bin1 = stagingBufferPool.bins()[0];
            const auto &bin2 = stagingBufferPool.bins()[1];

            CHECK(bin1.buffer.isValid());
            CHECK(bin2.buffer.isValid());
            CHECK(bin1.buffer.handle() == r1.second);
            CHECK(bin2.buffer.handle() == r2.second);
            CHECK(bin1.isMapped == false);
            CHECK(bin2.isMapped == true);
            CHECK(bin1.m_allocations.size() == 1);
            CHECK(bin2.m_allocations.size() == 1);
            CHECK(bin1.m_allocations[0].offset == 0);
            CHECK(bin1.m_allocations[0].offset == r1.first);
            CHECK(bin1.m_allocations[0].size == 512);
            CHECK(bin2.m_allocations[0].offset == 0);
            CHECK(bin2.m_allocations[0].offset == r2.first);
            CHECK(bin2.m_allocations[0].size == 768);

            // If we could map bin1
            // CHECK(std::memcmp(bin1.mapped, smallTestData.data(), 512) == 0);
            CHECK(std::memcmp(bin2.mapped, bigTestData.data(), 768) == 0);
        }

        SUBCASE("Reuse existing bin if content fits")
        {
            // GIVEN
            KDGpuUtils::ResourceDeleter deleter(&device, MAX_FRAMES_IN_FLIGHT);
            KDGpuUtils::StagingBufferPoolImpl<1, 1024> stagingBufferPool(&device, &deleter);

            // WHEN
            const std::vector<uint8_t> smallTestData = std::vector<uint8_t>(512, 0xaa);
            const std::vector<uint8_t> bigTestData = std::vector<uint8_t>(768, 0xee);
            const std::pair<size_t, KDGpu::Handle<KDGpu::Buffer_t>> r1 = stagingBufferPool.stage(smallTestData);
            const std::pair<size_t, KDGpu::Handle<KDGpu::Buffer_t>> r2 = stagingBufferPool.stage(bigTestData);
            const std::pair<size_t, KDGpu::Handle<KDGpu::Buffer_t>> r3 = stagingBufferPool.stage(smallTestData);

            // THEN
            REQUIRE(stagingBufferPool.bins().size() == 2);
            const auto &bin1 = stagingBufferPool.bins()[0];
            const auto &bin2 = stagingBufferPool.bins()[1];

            CHECK(bin1.buffer.isValid());
            CHECK(bin2.buffer.isValid());
            CHECK(bin1.buffer.handle() == r1.second);
            CHECK(bin1.buffer.handle() == r3.second);
            CHECK(bin2.buffer.handle() == r2.second);
            CHECK(bin1.isMapped == true);
            CHECK(bin2.isMapped == false);
            CHECK(bin1.m_allocations.size() == 2);
            CHECK(bin2.m_allocations.size() == 1);
            CHECK(bin1.m_allocations[0].offset == 0);
            CHECK(bin1.m_allocations[0].offset == r1.first);
            CHECK(bin1.m_allocations[0].size == 512);
            CHECK(bin1.m_allocations[1].offset == 512);
            CHECK(bin1.m_allocations[1].offset == r3.first);
            CHECK(bin1.m_allocations[1].size == 512);
            CHECK(bin2.m_allocations[0].offset == 0);
            CHECK(bin2.m_allocations[0].offset == r2.first);
            CHECK(bin2.m_allocations[0].size == 768);

            CHECK(std::memcmp(bin1.mapped, smallTestData.data(), 512) == 0);
            CHECK(std::memcmp(reinterpret_cast<uint8_t *>(bin1.mapped) + 512, smallTestData.data(), 512) == 0);

            // If we could map bin2
            // CHECK(std::memcmp(bin2.mapped, bigTestData.data(), 768) == 0);
        }

        SUBCASE("Different bins for different frames in flight")
        {
            // GIVEN
            KDGpuUtils::ResourceDeleter deleter(&device, MAX_FRAMES_IN_FLIGHT);
            KDGpuUtils::StagingBufferPoolImpl<1, 1024> stagingBufferPool(&device, &deleter);

            // WHEN
            const std::vector<uint8_t> smallTestData = std::vector<uint8_t>(512, 0xaa);
            const std::pair<size_t, KDGpu::Handle<KDGpu::Buffer_t>> r1 = stagingBufferPool.stage(smallTestData);

            // THEN
            REQUIRE(stagingBufferPool.bins().size() == 1);
            const auto &bin1 = stagingBufferPool.bins()[0];

            CHECK(bin1.buffer.isValid());
            CHECK(bin1.buffer.handle() == r1.second);
            CHECK(bin1.isMapped == true);
            CHECK(bin1.frameIndex == 0);

            // WHEN
            stagingBufferPool.flush();

            // THEN
            CHECK(bin1.isMapped == false);

            // WHEN
            stagingBufferPool.derefFrameIndex(1);
            const std::pair<size_t, KDGpu::Handle<KDGpu::Buffer_t>> r2 = stagingBufferPool.stage(smallTestData);

            // THEN
            REQUIRE(stagingBufferPool.bins().size() == 2);
            const auto &bin2 = stagingBufferPool.bins()[1];
            CHECK(bin2.frameIndex == 1);
            CHECK(bin2.isMapped == true);
        }
    }

    TEST_CASE("Trims when moving to next frame")
    {
        SUBCASE("Flush")
        {
            // GIVEN
            KDGpuUtils::ResourceDeleter deleter(&device, MAX_FRAMES_IN_FLIGHT);
            KDGpuUtils::StagingBufferPoolImpl stagingBufferPool(&device, &deleter);

            // WHEN
            const std::vector<uint8_t> smallTestData = std::vector<uint8_t>(512, 0xaa);
            stagingBufferPool.stage(smallTestData);

            // THEN
            REQUIRE(stagingBufferPool.bins().size() == 1);
            const auto &bin = stagingBufferPool.bins()[0];
            CHECK(bin.buffer.isValid());
            CHECK(bin.isMapped == true);

            // WHEN
            stagingBufferPool.flush();

            // THEN
            CHECK(bin.isMapped == false);
        }

        SUBCASE("Clear allocations on bin when moving to next Frame")
        {
            // GIVEN
            KDGpuUtils::ResourceDeleter deleter(&device, MAX_FRAMES_IN_FLIGHT);
            KDGpuUtils::StagingBufferPoolImpl stagingBufferPool(&device, &deleter);

            // WHEN
            const std::vector<uint8_t> smallTestData = std::vector<uint8_t>(512, 0xaa);
            stagingBufferPool.stage(smallTestData);
            stagingBufferPool.stage(smallTestData);
            stagingBufferPool.stage(smallTestData);
            stagingBufferPool.stage(smallTestData);
            stagingBufferPool.stage(smallTestData);

            // THEN
            REQUIRE(stagingBufferPool.bins().size() == 1);
            const auto &bin = stagingBufferPool.bins()[0];
            CHECK(bin.m_allocations.size() == 5);

            // WHEN
            stagingBufferPool.flush();
            stagingBufferPool.moveToNextFrame();

            // THEN
            CHECK(stagingBufferPool.bins().size() == 1);
            CHECK(bin.isMapped == false);
            CHECK(bin.m_allocations.empty());
        }

        SUBCASE("Destroys excess bins")
        {
            // GIVEN
            KDGpuUtils::ResourceDeleter deleter(&device, MAX_FRAMES_IN_FLIGHT);
            KDGpuUtils::StagingBufferPoolImpl<2, 512> stagingBufferPool(&device, &deleter);

            // WHEN
            const std::vector<uint8_t> smallTestData = std::vector<uint8_t>(512, 0xaa);
            stagingBufferPool.stage(smallTestData);
            stagingBufferPool.stage(smallTestData);
            stagingBufferPool.stage(smallTestData);
            stagingBufferPool.stage(smallTestData);
            stagingBufferPool.stage(smallTestData);

            // THEN
            REQUIRE(stagingBufferPool.bins().size() == 5);

            // WHEN
            stagingBufferPool.flush();
            stagingBufferPool.moveToNextFrame();

            // THEN
            REQUIRE(stagingBufferPool.bins().size() == 2);
            auto &bins = deleter.frameBins();
            CHECK(bins.size() == 1);
            CHECK(bins[0].resources.get<KDGpu::Buffer>().size() == 3);
        }

        SUBCASE("Destroys excess bins per swapchain image")
        {
            // GIVEN
            KDGpuUtils::ResourceDeleter deleter(&device, MAX_FRAMES_IN_FLIGHT);
            KDGpuUtils::StagingBufferPoolImpl<2, 512> stagingBufferPool(&device, &deleter);

            // WHEN
            const std::vector<uint8_t> smallTestData = std::vector<uint8_t>(512, 0xaa);
            stagingBufferPool.stage(smallTestData);
            stagingBufferPool.stage(smallTestData);
            stagingBufferPool.stage(smallTestData);
            stagingBufferPool.stage(smallTestData);
            stagingBufferPool.stage(smallTestData);

            stagingBufferPool.flush();
            stagingBufferPool.derefFrameIndex(1);

            stagingBufferPool.stage(smallTestData);
            stagingBufferPool.stage(smallTestData);
            stagingBufferPool.stage(smallTestData);
            stagingBufferPool.stage(smallTestData);
            stagingBufferPool.stage(smallTestData);

            stagingBufferPool.flush();
            stagingBufferPool.derefFrameIndex(2);

            stagingBufferPool.stage(smallTestData);
            stagingBufferPool.stage(smallTestData);
            stagingBufferPool.stage(smallTestData);
            stagingBufferPool.stage(smallTestData);
            stagingBufferPool.stage(smallTestData);

            stagingBufferPool.flush();

            // THEN
            REQUIRE(stagingBufferPool.bins().size() == 15);

            // WHEN
            stagingBufferPool.moveToNextFrame();

            // THEN
            REQUIRE(stagingBufferPool.bins().size() == 6);
            CHECK(stagingBufferPool.bins()[0].frameIndex == 0);
            CHECK(stagingBufferPool.bins()[1].frameIndex == 0);
            CHECK(stagingBufferPool.bins()[2].frameIndex == 1);
            CHECK(stagingBufferPool.bins()[3].frameIndex == 1);
            CHECK(stagingBufferPool.bins()[4].frameIndex == 2);
            CHECK(stagingBufferPool.bins()[5].frameIndex == 2);

            auto &bins = deleter.frameBins();
            CHECK(bins.size() == 1);
            CHECK(bins[0].resources.get<KDGpu::Buffer>().size() == 9);
        }
    }
}

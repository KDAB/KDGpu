/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpuUtils/resource_deleter.h>

#include <KDGpu/buffer_options.h>
#include <KDGpu/device.h>
#include <KDGpu/instance.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

#include <algorithm>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

TEST_SUITE("ResourceDeleter")
{
    std::unique_ptr<KDGpu::GraphicsApi> api = std::make_unique<KDGpu::VulkanGraphicsApi>();
    KDGpu::Instance instance = api->createInstance(KDGpu::InstanceOptions{
            .applicationName = "ResourceDeleter",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });
    KDGpu::Adapter *discreteGPUAdapter = instance.selectAdapter(KDGpu::AdapterDeviceType::Default);
    KDGpu::Device device = discreteGPUAdapter->createDevice();

    constexpr size_t MAX_FRAMES_IN_FLIGHT = 2;

    TEST_CASE("Creation")
    {
        SUBCASE("can create a resource deleter")
        {
            KDGpuUtils::ResourceDeleter deleter(&device, MAX_FRAMES_IN_FLIGHT);

            REQUIRE(deleter.frameBins().empty() == true);
        }
    }

    TEST_CASE("Adding resources to be deleted")
    {
        SUBCASE("can schedule a buffer resource to be deleted later")
        {
            // GIVEN
            KDGpuUtils::ResourceDeleter deleter(&device, MAX_FRAMES_IN_FLIGHT);

            // Create a buffer
            const KDGpu::DeviceSize bufferSize = 1024;
            KDGpu::Buffer buffer = device.createBuffer(KDGpu::BufferOptions{
                    .size = bufferSize,
                    .usage = KDGpu::BufferUsageFlagBits::VertexBufferBit,
                    .memoryUsage = KDGpu::MemoryUsage::CpuToGpu,
            });
            const auto bufHandle = buffer.handle();

            // WHEN
            // Schedule it for deletion after max in flight frame count frames have been processed
            deleter.deleteLater(std::move(buffer));

            // THEN
            auto &bins = deleter.frameBins();
            REQUIRE(bins.size() == 1);

            auto &bin = bins.front();
            REQUIRE(bin.frameNumber == deleter.frameNumber());

            const auto maxFrameCount = MAX_FRAMES_IN_FLIGHT;
            REQUIRE(bin.frameReferences.size() == maxFrameCount);
            REQUIRE(std::all_of(bin.frameReferences.begin(), bin.frameReferences.end(), [](bool b) { return b == true; }) == true);

            // REQUIRE(bin.resources.get<Render::ImageSampler>().empty() == true);
            const auto &buffers = bin.resources.get<KDGpu::Buffer>();
            REQUIRE(buffers.size() == 1);
            REQUIRE(buffers[0].handle() == bufHandle);
        }

        SUBCASE("can schedule multiple buffer resources to be deleted later")
        {
            // GIVEN
            KDGpuUtils::ResourceDeleter deleter(&device, MAX_FRAMES_IN_FLIGHT);

            // Create several buffers
            const KDGpu::DeviceSize bufferSize = 1024;
            const size_t bufferCount = 5;
            std::vector<KDGpu::Handle<KDGpu::Buffer_t>> handles;

            // WHEN
            for (size_t i = 0; i < bufferCount; ++i) {
                KDGpu::Buffer buffer = device.createBuffer(KDGpu::BufferOptions{
                        .size = bufferSize,
                        .usage = KDGpu::BufferUsageFlagBits::VertexBufferBit,
                        .memoryUsage = KDGpu::MemoryUsage::CpuToGpu,
                });

                // Record buffer handles for later comparisons
                handles.push_back(buffer.handle());

                // Schedule them for deletion after max in flight frame count frames have been processed
                deleter.deleteLater(std::move(buffer));
            }

            // THEN
            auto &bins = deleter.frameBins();
            REQUIRE(bins.size() == 1);

            auto &bin = bins.front();
            REQUIRE(bin.frameNumber == deleter.frameNumber());

            const auto maxFrameCount = MAX_FRAMES_IN_FLIGHT;
            REQUIRE(bin.frameReferences.size() == maxFrameCount);
            REQUIRE(std::all_of(bin.frameReferences.begin(), bin.frameReferences.end(), [](bool b) { return b == true; }) == true);

            // REQUIRE(bin.resources.get<Render::ImageSampler>().empty() == true);
            const auto &buffers = bin.resources.get<KDGpu::Buffer>();
            REQUIRE(buffers.size() == handles.size());
            for (size_t i = 0; i < handles.size(); ++i) {
                REQUIRE(buffers[i].handle() == handles[i]);
            }
        }
    }

    TEST_CASE("Incrementing the frame number")
    {
        SUBCASE("can schedule a buffer resource from multiple frames to be deleted later")
        {
            // GIVEN
            KDGpuUtils::ResourceDeleter deleter(&device, MAX_FRAMES_IN_FLIGHT);

            // Create several buffers
            const KDGpu::DeviceSize bufferSize = 1024;
            const size_t frameCount = 5;
            uint64_t initialFrameNumber = 0;
            std::vector<KDGpu::Handle<KDGpu::Buffer_t>> handles;

            // WHEN
            for (size_t i = 0; i < frameCount; ++i) {
                KDGpu::Buffer buffer = device.createBuffer(KDGpu::BufferOptions{
                        .size = bufferSize,
                        .usage = KDGpu::BufferUsageFlags(KDGpu::BufferUsageFlagBits::VertexBufferBit),
                        .memoryUsage = KDGpu::MemoryUsage::CpuToGpu });

                // Record buffer handles for later comparisons
                handles.push_back(buffer.handle());

                // Schedule them for deletion after max in flight frame count frames have been processed
                deleter.deleteLater(std::move(buffer));

                // Move to the next frame
                deleter.moveToNextFrame();
            }

            // THEN
            auto &bins = deleter.frameBins();
            REQUIRE(bins.size() == frameCount);
            for (size_t i = 0; i < handles.size(); ++i) {
                auto &bin = bins[i];
                REQUIRE(bin.frameNumber == initialFrameNumber + i);

                const auto maxFrameCount = MAX_FRAMES_IN_FLIGHT;
                REQUIRE(bin.frameReferences.size() == maxFrameCount);
                REQUIRE(std::all_of(bin.frameReferences.begin(), bin.frameReferences.end(), [](bool b) { return b == true; }) == true);

                // const auto &imageSamplers = bin.resources.get<Render::ImageSampler>();
                // REQUIRE(imageSamplers.empty() == true);

                const auto &buffers = bin.resources.get<KDGpu::Buffer>();
                REQUIRE(buffers.size() == 1);
                REQUIRE(buffers[0].handle() == handles[i]);
            }
        }
    }

    TEST_CASE("Releasing Frame References")
    {
        SUBCASE("a buffer resource is deleted once all frames in flight have dereffed it")
        {
            // GIVEN
            KDGpuUtils::ResourceDeleter deleter(&device, MAX_FRAMES_IN_FLIGHT);

            // Create a buffer
            const KDGpu::DeviceSize bufferSize = 1024;

            KDGpu::Buffer buffer = device.createBuffer(KDGpu::BufferOptions{
                    .size = bufferSize,
                    .usage = KDGpu::BufferUsageFlags(KDGpu::BufferUsageFlagBits::VertexBufferBit),
                    .memoryUsage = KDGpu::MemoryUsage::CpuToGpu,
            });

            // WHEN
            // Schedule it for deletion after max in flight frame count frames have been processed
            deleter.deleteLater(std::move(buffer));

            // THEN
            auto &bins = deleter.frameBins();
            REQUIRE(bins.size() == 1);
            REQUIRE(bins.front().resources.get<KDGpu::Buffer>().size() == 1);

            // WHEN
            // Now release the references one at a time for the current frameNumber
            const auto maxFrameCount = MAX_FRAMES_IN_FLIGHT;
            for (auto i = 0; i < maxFrameCount; ++i) {
                REQUIRE(bins.size() == 1);
                deleter.derefFrameIndex(i);
            }

            // THEN
            REQUIRE(bins.size() == 1);

            // WHEN
            // Move to next frame and release the references one at a time
            deleter.moveToNextFrame();
            for (auto i = 0; i < maxFrameCount; ++i) {
                REQUIRE(bins.size() == 1);
                deleter.derefFrameIndex(i);
            }

            // THEN
            // Bin and the contained buffer should be deleted now all refs are released
            REQUIRE(bins.empty());
        }

        SUBCASE("multiple buffers from a single frame are deleted when all frames have dereffed the frame")
        {
            // GIVEN
            KDGpuUtils::ResourceDeleter deleter(&device, MAX_FRAMES_IN_FLIGHT);

            // Create a buffer
            const KDGpu::DeviceSize bufferSize = 1024;
            const size_t bufferCount = 5;

            // WHEN
            for (size_t i = 0; i < bufferCount; ++i) {
                KDGpu::Buffer buffer = device.createBuffer(KDGpu::BufferOptions{
                        .size = bufferSize,
                        .usage = KDGpu::BufferUsageFlags(KDGpu::BufferUsageFlagBits::VertexBufferBit),
                        .memoryUsage = KDGpu::MemoryUsage::CpuToGpu,
                });

                // Schedule it for deletion after max in flight frame count frames have been processed
                deleter.deleteLater(std::move(buffer));
            }

            // THEN
            auto &bins = deleter.frameBins();
            REQUIRE(bins.size() == 1);
            REQUIRE(bins.front().resources.get<KDGpu::Buffer>().size() == bufferCount);

            // WHEN
            // Now release the references one at a time
            const auto maxFrameCount = MAX_FRAMES_IN_FLIGHT;
            for (auto i = 0; i < maxFrameCount; ++i) {
                REQUIRE(bins.size() == 1);
                deleter.derefFrameIndex(i);
            }

            // THEN
            REQUIRE(bins.size() == 1);

            // WHEN
            deleter.moveToNextFrame();
            for (auto i = 0; i < maxFrameCount; ++i) {
                REQUIRE(bins.size() == 1);
                deleter.derefFrameIndex(i);
            }

            // THEN
            // Bin and the contained buffer should be deleted now all refs are released
            REQUIRE(bins.empty());
        }

        SUBCASE("a single buffer from multiple frames are deleted when all frames have dereffed the corresponding frame")
        {
            // GIVEN
            KDGpuUtils::ResourceDeleter deleter(&device, MAX_FRAMES_IN_FLIGHT);

            // Create a buffer
            const KDGpu::DeviceSize bufferSize = 1024;
            const size_t frameCount = 5;

            // WHEN
            for (size_t i = 0; i < frameCount; ++i) {
                KDGpu::Buffer buffer = device.createBuffer(KDGpu::BufferOptions{
                        .size = bufferSize,
                        .usage = KDGpu::BufferUsageFlags(KDGpu::BufferUsageFlagBits::VertexBufferBit),
                        .memoryUsage = KDGpu::MemoryUsage::CpuToGpu });

                // Schedule it for deletion after max in flight frame count frames have been processed
                deleter.deleteLater(std::move(buffer));

                // Move to the next frame
                deleter.moveToNextFrame();
            }

            // THEN
            auto &bins = deleter.frameBins();
            REQUIRE(bins.size() == frameCount);

            // WHEN
            // Now release the references one at a time
            const auto maxFrameCount = MAX_FRAMES_IN_FLIGHT;
            for (auto i = 0; i < maxFrameCount; ++i) {
                REQUIRE(bins.size() == frameCount);

                deleter.derefFrameIndex(i);
            }

            // THEN
            // Bin and the contained buffer should be deleted now all refs are released
            REQUIRE(bins.empty());
        }

        SUBCASE("multiple buffers from multiple frames are deleted when all images have dereffed the corresponding frame")
        {
            // GIVEN
            KDGpuUtils::ResourceDeleter deleter(&device, MAX_FRAMES_IN_FLIGHT);

            // Create a buffer
            const KDGpu::DeviceSize bufferSize = 1024;
            const size_t frameCount = 3;
            const size_t bufferCount = 10;

            // WHEN
            for (size_t i = 0; i < frameCount; ++i) {
                for (size_t j = 0; j < bufferCount; ++j) {
                    KDGpu::Buffer buffer = device.createBuffer(KDGpu::BufferOptions{
                            .size = bufferSize,
                            .usage = KDGpu::BufferUsageFlags(KDGpu::BufferUsageFlagBits::VertexBufferBit),
                            .memoryUsage = KDGpu::MemoryUsage::CpuToGpu });

                    // Schedule it for deletion after max in flight frame count frames have been processed
                    deleter.deleteLater(std::move(buffer));
                }

                // Move to the next frame
                deleter.moveToNextFrame();
            }

            // THEN
            auto &bins = deleter.frameBins();
            REQUIRE(bins.size() == frameCount);

            // WHEN
            // Now release the references one at a time
            const auto maxFrameCount = MAX_FRAMES_IN_FLIGHT;
            for (auto i = 0; i < maxFrameCount; ++i) {
                REQUIRE(bins.size() == frameCount);

                deleter.derefFrameIndex(i);
            }

            // THEN
            // Bin and the contained buffer should be deleted now all refs are released
            REQUIRE(bins.empty());
        }

        SUBCASE("realistic use case - deref as we progress through frames")
        {
            // GIVEN
            KDGpuUtils::ResourceDeleter deleter(&device, MAX_FRAMES_IN_FLIGHT);

            const KDGpu::DeviceSize bufferSize = 1024;
            const size_t bufferCount = 10;
            const auto maxFrameCount = MAX_FRAMES_IN_FLIGHT;
            const size_t frameCount = MAX_FRAMES_IN_FLIGHT + 5;
            KDGpu::Device &dev = device;

            auto createFrameResources = [&dev, bufferCount, bufferSize, &deleter]() {
                for (size_t j = 0; j < bufferCount; ++j) {
                    KDGpu::Buffer buffer = device.createBuffer(KDGpu::BufferOptions{
                            .size = bufferSize,
                            .usage = KDGpu::BufferUsageFlags(KDGpu::BufferUsageFlagBits::VertexBufferBit),
                            .memoryUsage = KDGpu::MemoryUsage::CpuToGpu });

                    // Schedule it for deletion after max in flight frame count frames have been processed
                    deleter.deleteLater(std::move(buffer));
                }
            };

            // WHEN
            auto &bins = deleter.frameBins();
            for (size_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
                // Create some resources
                createFrameResources();

                // Move to the next frame
                deleter.moveToNextFrame();

                const auto expectedBinCountBeforeDeref = maxFrameCount - size_t(std::max(int(maxFrameCount) - int(frameIndex) - 1, int(0)));
                REQUIRE(bins.size() == expectedBinCountBeforeDeref);

                // Deref image 0 - should not delete anything
                deleter.derefFrameIndex(frameIndex % maxFrameCount);

                // Example sequence with 3 frames in flight
                //
                // Frame 12345:
                //      bins[0]: 12345 : false true true
                //
                // Frame 12346
                //      bins[0]: 12345 : false false true
                //      bins[1]: 12346 : true  false true
                //
                // Frame 12347
                //      bins[0]: 12345 : false false false => delete this bin
                //      bins[1]: 12346 : true  false false
                //      bins[2]: 12347 : true  true  false
                //
                // Frame 12348
                //      bins[0]: 12346 : false false false => delete this bin
                //      bins[1]: 12347 : false false false
                //      bins[2]: 12348 : false true  false

                const auto expectedBinCountAfterDeref = maxFrameCount - size_t(std::max(int(maxFrameCount) - int(frameIndex) - 1, int(1)));
                REQUIRE(bins.size() == expectedBinCountAfterDeref);
            }

            // THEN
            // We expect there to be maxFrameCount - 1 bins remaining once we stop adding more stuff.i.e. the steady state
            REQUIRE(bins.size() == maxFrameCount - 1);

            // WHEN
            // Release all image refs
            for (size_t i = 0; i < maxFrameCount; ++i)
                deleter.derefFrameIndex(i);

            // THEN
            // Bin and the contained buffer should be deleted now all refs are released
            REQUIRE(bins.empty());
        }
    }
}

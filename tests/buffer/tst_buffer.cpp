/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/config.h>
#include <KDGpu/buffer.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/device.h>
#include <KDGpu/instance.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

#include <set>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDGpu;

TEST_SUITE("Buffer")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "buffer",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);
    // CHECK(discreteGPUAdapter.isValid());
    Device device = discreteGPUAdapter->createDevice();
    // CHECK(device.isValid());

    TEST_CASE("Construction")
    {
        SUBCASE("A default constructed Buffer is invalid")
        {
            // GIVEN
            Buffer buffer;
            // THEN
            REQUIRE(!buffer.isValid());
        }

        SUBCASE("A constructed Buffer from a Vulkan API with no initial data")
        {
            // GIVEN
            const BufferOptions bufferOptions = {
                .size = 4 * sizeof(float),
                .usage = BufferUsageFlagBits::VertexBufferBit,
                .memoryUsage = MemoryUsage::CpuToGpu
            };

            // WHEN
            Buffer b = device.createBuffer(bufferOptions);

            // THEN
            CHECK(b.isValid());
        }

        SUBCASE("A constructed Buffer from a Vulkan API with initial data")
        {
            // GIVEN
            const BufferOptions bufferOptions = {
                .size = 4 * sizeof(float),
                .usage = BufferUsageFlagBits::VertexBufferBit,
                .memoryUsage = MemoryUsage::CpuToGpu
            };

            const std::vector<float> vertexData = {
                1.0f, -1.0f, 0.0f, 1.0f
            };

            // WHEN
            Buffer b = device.createBuffer(bufferOptions, vertexData.data());

            // THEN
            CHECK(b.isValid());
            const MemoryHandle memoryHandle = b.externalMemoryHandle();
            CHECK(memoryHandle.allocationSize > 0);
        }

#if defined(KDGPU_PLATFORM_LINUX)
        SUBCASE("A constructed Buffer from a Vulkan API with external FD")
        {
            // GIVEN
            const BufferOptions bufferOptions = {
                .size = 4 * sizeof(float),
                .usage = BufferUsageFlagBits::VertexBufferBit,
                .memoryUsage = MemoryUsage::CpuToGpu,
                .externalMemoryHandleType = ExternalMemoryHandleTypeFlagBits::OpaqueFD,
            };

            // WHEN
            Buffer b = device.createBuffer(bufferOptions);

            // THEN
            CHECK(b.isValid());
            const MemoryHandle externalHandleOrFD = b.externalMemoryHandle();
            CHECK(std::get<int>(externalHandleOrFD.handle) > -1);
            CHECK(externalHandleOrFD.allocationSize > 0);
        }
#elif defined(KDGPU_PLATFORM_WIN32)
        SUBCASE("A constructed Buffer from a Vulkan API with external Handle")
        {
            // GIVEN
            const BufferOptions bufferOptions = {
                .size = 4 * sizeof(float),
                .usage = BufferUsageFlagBits::VertexBufferBit,
                .memoryUsage = MemoryUsage::CpuToGpu,
                .externalMemoryHandleType = ExternalMemoryHandleTypeFlagBits::OpaqueWin32,
            };

            // WHEN
            Buffer b = device.createBuffer(bufferOptions);

            // THEN
            CHECK(b.isValid());
            const MemoryHandle externalHandleOrFD = b.externalMemoryHandle();
            CHECK(std::get<HANDLE>(externalHandleOrFD.handle) != nullptr);
            CHECK(externalHandleOrFD.allocationSize > 0);
        }
#endif
    }

    TEST_CASE("Destruction")
    {
        // GIVEN
        const BufferOptions bufferOptions = {
            .size = 4 * sizeof(float),
            .usage = BufferUsageFlagBits::VertexBufferBit,
            .memoryUsage = MemoryUsage::CpuToGpu
        };

        const std::vector<float> vertexData = {
            1.0f, -1.0f, 0.0f, 1.0f
        };

        Handle<Buffer_t> bufferHandle;

        SUBCASE("Going Out Of Scope")
        {
            {
                // WHEN
                Buffer b = device.createBuffer(bufferOptions, vertexData.data());
                bufferHandle = b.handle();

                // THEN
                CHECK(b.isValid());
                CHECK(bufferHandle.isValid());
                CHECK(api->resourceManager()->getBuffer(bufferHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getBuffer(bufferHandle) == nullptr);
        }

        SUBCASE("Move assignment")
        {
            // WHEN
            Buffer b = device.createBuffer(bufferOptions, vertexData.data());
            bufferHandle = b.handle();

            // THEN
            CHECK(b.isValid());
            CHECK(bufferHandle.isValid());
            CHECK(api->resourceManager()->getBuffer(bufferHandle) != nullptr);

            // WHEN
            b = {};

            // THEN
            CHECK(api->resourceManager()->getBuffer(bufferHandle) == nullptr);
        }
    }

    TEST_CASE("Map/Unmap")
    {
        SUBCASE("Invalid Buffer")
        {
            // GIVEN
            Buffer buffer;

            // WHEN
            void *m = buffer.map();

            // THEN
            CHECK(m == nullptr);

            // WHEN
            buffer.unmap();

            // THEN -> Shouldn't crash
        }

        SUBCASE("Valid Buffer")
        {
            // GIVEN
            const BufferOptions bufferOptions = {
                .size = 4 * sizeof(float),
                .usage = BufferUsageFlagBits::VertexBufferBit,
                .memoryUsage = MemoryUsage::CpuToGpu
            };

            const std::vector<float> vertexData = {
                1.0f, -1.0f, 0.0f, 1.0f
            };

            // WHEN
            Buffer b = device.createBuffer(bufferOptions, vertexData.data());

            // THEN
            CHECK(b.isValid());

            // WHEN
            const float *rawData = reinterpret_cast<const float *>(b.map());

            // THEN
            CHECK(rawData != nullptr);
            CHECK(rawData[0] == vertexData[0]);
            CHECK(rawData[1] == vertexData[1]);
            CHECK(rawData[2] == vertexData[2]);
            CHECK(rawData[3] == vertexData[3]);

            // WHEN
            b.unmap();

            // THEN -> It's all good
        }

        SUBCASE("Flush")
        {
            // GIVEN
            const BufferOptions bufferOptions = {
                .size = 4 * sizeof(float),
                .usage = BufferUsageFlagBits::VertexBufferBit,
                .memoryUsage = MemoryUsage::CpuToGpu
            };

            // WHEN
            Buffer b = device.createBuffer(bufferOptions);

            // THEN
            CHECK(b.isValid());

            // WHEN
            const std::vector<float> vertexData = {
                1.0f, -1.0f, 0.0f, 1.0f
            };
            float *rawData = reinterpret_cast<float *>(b.map());
            std::memcpy(rawData, vertexData.data(), vertexData.size() * sizeof(float));
            b.unmap();
            b.flush();

            // THEN
            const float *rawData2 = reinterpret_cast<const float *>(b.map());

            CHECK(std::memcmp(rawData2, vertexData.data(), vertexData.size() * sizeof(float)) == 0);

            b.unmap();
        }

        SUBCASE("Invalidate")
        {
            // GIVEN
            const BufferOptions bufferOptions = {
                .size = 4 * sizeof(float),
                .usage = BufferUsageFlagBits::VertexBufferBit,
                .memoryUsage = MemoryUsage::CpuToGpu
            };

            // WHEN
            const std::vector<float> vertexData = {
                1.0f, -1.0f, 0.0f, 1.0f
            };
            Buffer b = device.createBuffer(bufferOptions, vertexData.data());

            // THEN
            CHECK(b.isValid());

            // WHEN
            b.invalidate();
            const float *rawData = reinterpret_cast<const float *>(b.map());

            // THEN
            CHECK(std::memcmp(rawData, vertexData.data(), vertexData.size() * sizeof(float)) == 0);

            b.unmap();
        }
    }

    TEST_CASE("Comparison")
    {
        SUBCASE("Compare default constructed Buffers")
        {
            // GIVEN
            Buffer a;
            Buffer b;

            // THEN
            CHECK(a == b);
        }

        SUBCASE("Compare device create buffers")
        {
            // GIVEN
            const BufferOptions bufferOptions = {
                .size = 4 * sizeof(float),
                .usage = BufferUsageFlagBits::VertexBufferBit,
                .memoryUsage = MemoryUsage::CpuToGpu
            };

            // WHEN
            Buffer b = device.createBuffer(bufferOptions);
            Buffer a = device.createBuffer(bufferOptions);

            // THEN
            CHECK(a != b);
        }
    }
}

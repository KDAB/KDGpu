/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/bind_group_pool.h>
#include <KDGpu/bind_group_pool_options.h>
#include <KDGpu/device.h>
#include <KDGpu/instance.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDGpu;

TEST_SUITE("BindGroupPool")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "BindGroupPool",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);
    Device device = discreteGPUAdapter->createDevice(DeviceOptions{
            .requestedFeatures = discreteGPUAdapter->features(),
    });

    TEST_CASE("Construction")
    {
        SUBCASE("A default constructed BindGroupPool is invalid")
        {
            // GIVEN
            BindGroupPool pool;
            // THEN
            REQUIRE(!pool.isValid());
        }

        SUBCASE("A constructed BindGroupPool from a Vulkan API")
        {
            // GIVEN
            const BindGroupPoolOptions poolOptions = {
                .label = "Test Pool",
                .uniformBufferCount = 10,
                .dynamicUniformBufferCount = 5,
                .storageBufferCount = 8,
                .textureSamplerCount = 6,
                .textureCount = 4,
                .samplerCount = 2,
                .imageCount = 3,
                .inputAttachmentCount = 1,
                .maxBindGroupCount = 50,
                .flags = BindGroupPoolFlagBits::CreateFreeBindGroups
            };

            // WHEN
            BindGroupPool pool = device.createBindGroupPool(poolOptions);

            // THEN
            CHECK(pool.isValid());
        }

        SUBCASE("A constructed BindGroupPool with minimal options")
        {
            // GIVEN
            const BindGroupPoolOptions poolOptions = {
                .maxBindGroupCount = 10
            };

            // WHEN
            BindGroupPool pool = device.createBindGroupPool(poolOptions);

            // THEN
            CHECK(pool.isValid());
        }
    }

    TEST_CASE("Reset BindGroupPool")
    {
        SUBCASE("Reset a valid BindGroupPool")
        {
            // GIVEN
            const BindGroupPoolOptions poolOptions = {
                .label = "Reset Test Pool",
                .uniformBufferCount = 5,
                .maxBindGroupCount = 20,
                .flags = BindGroupPoolFlagBits::CreateFreeBindGroups
            };

            BindGroupPool pool = device.createBindGroupPool(poolOptions);

            // THEN
            CHECK(pool.isValid());

            // WHEN
            pool.reset();

            // THEN - Pool should still be valid after reset
            CHECK(pool.isValid());
        }
    }

    TEST_CASE("Destruction")
    {
        // GIVEN
        const BindGroupPoolOptions poolOptions = {
            .label = "Destruction Test Pool",
            .uniformBufferCount = 3,
            .maxBindGroupCount = 15
        };

        Handle<BindGroupPool_t> poolHandle;

        SUBCASE("Going Out Of Scope")
        {
            {
                // WHEN
                BindGroupPool pool = device.createBindGroupPool(poolOptions);
                poolHandle = pool.handle();

                // THEN
                CHECK(pool.isValid());
                CHECK(poolHandle.isValid());
                CHECK(api->resourceManager()->getBindGroupPool(poolHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getBindGroupPool(poolHandle) == nullptr);
        }

        SUBCASE("Move assignment")
        {
            // WHEN
            BindGroupPool pool = device.createBindGroupPool(poolOptions);
            poolHandle = pool.handle();

            // THEN
            CHECK(pool.isValid());
            CHECK(poolHandle.isValid());
            CHECK(api->resourceManager()->getBindGroupPool(poolHandle) != nullptr);

            // WHEN
            pool = {};

            // THEN
            CHECK(api->resourceManager()->getBindGroupPool(poolHandle) == nullptr);
        }
    }

    TEST_CASE("Move Semantics")
    {
        // GIVEN
        const BindGroupPoolOptions poolOptions = {
            .label = "Move Test Pool",
            .uniformBufferCount = 4,
            .storageBufferCount = 2,
            .maxBindGroupCount = 25
        };

        Handle<BindGroupPool_t> poolHandle;

        SUBCASE("Move Constructor")
        {
            {
                // WHEN
                BindGroupPool pool1 = device.createBindGroupPool(poolOptions);
                poolHandle = pool1.handle();

                // THEN
                CHECK(pool1.isValid());
                CHECK(poolHandle.isValid());
                CHECK(api->resourceManager()->getBindGroupPool(poolHandle) != nullptr);

                // WHEN
                BindGroupPool pool2 = std::move(pool1);

                // THEN
                CHECK(!pool1.isValid()); // pool1 should be invalid after move
                CHECK(pool2.isValid()); // pool2 should be valid
                CHECK(pool2.handle() == poolHandle);
                CHECK(api->resourceManager()->getBindGroupPool(poolHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getBindGroupPool(poolHandle) == nullptr);
        }

        SUBCASE("Move Assignment")
        {
            // WHEN
            BindGroupPool pool1 = device.createBindGroupPool(poolOptions);
            poolHandle = pool1.handle();

            // THEN
            CHECK(pool1.isValid());
            CHECK(poolHandle.isValid());
            CHECK(api->resourceManager()->getBindGroupPool(poolHandle) != nullptr);

            // WHEN
            BindGroupPool pool2;
            CHECK(!pool2.isValid());

            pool2 = std::move(pool1);

            // THEN
            CHECK(!pool1.isValid()); // pool1 should be invalid after move
            CHECK(pool2.isValid()); // pool2 should be valid
            CHECK(pool2.handle() == poolHandle);
            CHECK(api->resourceManager()->getBindGroupPool(poolHandle) != nullptr);
        }
    }

    TEST_CASE("Pool Configuration Options")
    {
        SUBCASE("Pool with all resource types configured")
        {
            // GIVEN
            const BindGroupPoolOptions poolOptions = {
                .label = "Full Configuration Pool",
                .uniformBufferCount = 100,
                .dynamicUniformBufferCount = 50,
                .storageBufferCount = 75,
                .textureSamplerCount = 60,
                .textureCount = 40,
                .samplerCount = 20,
                .imageCount = 30,
                .inputAttachmentCount = 10,
                .maxBindGroupCount = 200,
                .flags = BindGroupPoolFlagBits::CreateFreeBindGroups
            };

            // WHEN
            BindGroupPool pool = device.createBindGroupPool(poolOptions);

            // THEN
            CHECK(pool.isValid());
        }

        SUBCASE("Pool with zero counts (should still be valid)")
        {
            // GIVEN
            const BindGroupPoolOptions poolOptions = {
                .label = "Zero Configuration Pool",
                .uniformBufferCount = 0,
                .dynamicUniformBufferCount = 0,
                .storageBufferCount = 0,
                .textureSamplerCount = 0,
                .textureCount = 0,
                .samplerCount = 0,
                .imageCount = 0,
                .inputAttachmentCount = 0,
                .maxBindGroupCount = 1, // Must have at least 1 max bind group
                .flags = BindGroupPoolFlagBits::CreateFreeBindGroups
            };

            // WHEN
            BindGroupPool pool = device.createBindGroupPool(poolOptions);

            // THEN
            CHECK(pool.isValid());
        }
    }

    TEST_CASE("Comparison")
    {
        SUBCASE("Compare default constructed BindGroupPools")
        {
            // GIVEN
            BindGroupPool a;
            BindGroupPool b;

            // THEN
            CHECK(a == b);
        }

        SUBCASE("Compare device created BindGroupPools")
        {
            // GIVEN
            const BindGroupPoolOptions poolOptions = {
                .label = "Comparison Test Pool",
                .uniformBufferCount = 5,
                .maxBindGroupCount = 10
            };

            // WHEN
            BindGroupPool a = device.createBindGroupPool(poolOptions);
            BindGroupPool b = device.createBindGroupPool(poolOptions);

            // THEN
            CHECK(a != b); // Different pools should not be equal
        }

        SUBCASE("Compare moved BindGroupPools")
        {
            // GIVEN
            const BindGroupPoolOptions poolOptions = {
                .label = "Move Comparison Test Pool",
                .uniformBufferCount = 3,
                .maxBindGroupCount = 8
            };

            // WHEN
            BindGroupPool original = device.createBindGroupPool(poolOptions);
            BindGroupPool moved = std::move(original);

            // THEN
            CHECK(original != moved); // Original should be invalid, moved should be valid
        }
    }

    TEST_CASE("Handle Operations")
    {
        SUBCASE("Handle retrieval and conversion")
        {
            // GIVEN
            const BindGroupPoolOptions poolOptions = {
                .label = "Handle Test Pool",
                .uniformBufferCount = 2,
                .maxBindGroupCount = 5
            };

            // WHEN
            BindGroupPool pool = device.createBindGroupPool(poolOptions);

            // THEN
            CHECK(pool.isValid());
            CHECK(pool.handle().isValid());

            // Test implicit conversion to handle
            Handle<BindGroupPool_t> handle = pool;
            CHECK(handle.isValid());
            CHECK(handle == pool.handle());
        }

        SUBCASE("Invalid pool handle operations")
        {
            // GIVEN
            BindGroupPool pool;

            // THEN
            CHECK(!pool.isValid());
            CHECK(!pool.handle().isValid());

            // Test implicit conversion to handle
            Handle<BindGroupPool_t> handle = pool;
            CHECK(!handle.isValid());
        }
    }
}

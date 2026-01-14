/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2026 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

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

class MockAdapter : public Adapter
{
public:
    MockAdapter() = default;

    explicit MockAdapter(GraphicsApi *api, const Handle<Adapter_t> &adapter)
        : Adapter(api, adapter)
    {
    }
};

TEST_SUITE("Adapter")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "adapter",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });
    VulkanInstance *apiInstance = api->resourceManager()->getInstance(instance);

    TEST_CASE("Construction")
    {
        SUBCASE("A default constructed Adapter is invalid")
        {
            // GIVEN
            Adapter adapter;
            // THEN
            REQUIRE(!adapter.isValid());
        }

        SUBCASE("A constructed Adapter from a Vulkan API")
        {
            // GIVEN
            const std::vector<Handle<Adapter_t>> vulkanAdapters = apiInstance->queryAdapters(instance);

            // THEN
            CHECK(vulkanAdapters.size() >= 0);

            // WHEN
            MockAdapter adapter{ api.get(), vulkanAdapters.front() };

            // THEN
            CHECK(adapter.isValid());
        }
    }

    TEST_CASE("Destruction")
    {
        // GIVEN
        Handle<Adapter_t> adapterHandle;

        SUBCASE("Going Out Of Scope")
        {
            {
                // WHEN
                const std::vector<Handle<Adapter_t>> vulkanAdapters = apiInstance->queryAdapters(instance);
                MockAdapter adapter{ api.get(), vulkanAdapters.front() };
                adapterHandle = adapter.handle();

                // THEN
                CHECK(adapter.isValid());
                CHECK(adapterHandle.isValid());
                CHECK(api->resourceManager()->getAdapter(adapterHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getAdapter(adapterHandle) == nullptr);
        }

        SUBCASE("Move assignment")
        {
            // WHEN
            const std::vector<Handle<Adapter_t>> vulkanAdapters = apiInstance->queryAdapters(instance);
            MockAdapter adapter{ api.get(), vulkanAdapters.front() };
            adapterHandle = adapter.handle();

            // THEN
            CHECK(adapter.isValid());
            CHECK(adapterHandle.isValid());
            CHECK(api->resourceManager()->getAdapter(adapterHandle) != nullptr);

            // WHEN
            adapter = {};

            // THEN
            CHECK(api->resourceManager()->getAdapter(adapterHandle) == nullptr);
        }
    }

    TEST_CASE("createDevice")
    {
        // GIVEN
        const std::vector<Handle<Adapter_t>> vulkanAdapters = apiInstance->queryAdapters(instance);
        MockAdapter adapter{ api.get(), vulkanAdapters.front() };

        // WHEN
        Device device = adapter.createDevice();

        // THEN
        CHECK(device.isValid());
    }

    TEST_CASE("Format Properties")
    {
        // GIVEN
        const std::vector<Handle<Adapter_t>> vulkanAdapters = apiInstance->queryAdapters(instance);
        MockAdapter adapter{ api.get(), vulkanAdapters.front() };

        // WHEN
        FormatProperties formatProperties = adapter.formatProperties(Format::R8G8B8A8_UNORM);

        // THEN
        CHECK(formatProperties.linearTilingFeatures.testFlag(FormatFeatureFlagBit::BlitDstBit));
        CHECK(formatProperties.optimalTilingFeatures.testFlag(FormatFeatureFlagBit::ColorAttachmentBit));
        CHECK(formatProperties.bufferFeatures.testFlag(FormatFeatureFlagBit::UniformTexelBufferBit));
    }

    TEST_CASE("supportsBlitting")
    {
        // GIVEN
        const std::vector<Handle<Adapter_t>> vulkanAdapters = apiInstance->queryAdapters(instance);
        MockAdapter adapter{ api.get(), vulkanAdapters.front() };

        // WHEN
        bool supportsBlitting1 = adapter.supportsBlitting(Format::R8G8B8A8_UNORM, TextureTiling::Optimal);
        bool supportsBlitting2 = adapter.supportsBlitting(Format::R8G8B8A8_UNORM, TextureTiling::Linear,
                                                          Format::R8G8B8A8_UNORM, TextureTiling::Optimal);

        // THEN
        CHECK(supportsBlitting1 == true);
        CHECK(supportsBlitting2 == true);
    }

    TEST_CASE("drmFormatModifierProperties")
    {
        // GIVEN
        const std::vector<Handle<Adapter_t>> vulkanAdapters = apiInstance->queryAdapters(instance);
        MockAdapter adapter{ api.get(), vulkanAdapters.front() };

        // WHEN
        std::vector<DrmFormatModifierProperties> drmFormatModifiers =
                adapter.drmFormatModifierProperties(Format::R8G8B8A8_UNORM);

        // THEN
        CHECK(!drmFormatModifiers.empty());
    }
}

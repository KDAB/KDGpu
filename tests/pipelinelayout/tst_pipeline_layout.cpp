/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/pipeline_layout.h>
#include <KDGpu/pipeline_layout_options.h>
#include <KDGpu/device.h>
#include <KDGpu/instance.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDGpu;

TEST_SUITE("PipelineLayout")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "PipelineLayout",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);
    Device device = discreteGPUAdapter->createDevice();

    TEST_CASE("Construction")
    {
        SUBCASE("A default constructed PipelineLayout is invalid")
        {
            // GIVEN
            PipelineLayout t;
            // THEN
            REQUIRE(!t.isValid());
        }

        SUBCASE("A constructed PipelineLayout from a Vulkan API")
        {
            // GIVEN
            PipelineLayoutOptions pipelineLayoutOptions = {
                .bindGroupLayouts = {}
            };

            // WHEN
            PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);

            // THEN
            CHECK(pipelineLayout.isValid());
        }
    }

    TEST_CASE("Destruction")
    {
        // GIVEN
        PipelineLayoutOptions pipelineLayoutOptions = {
            .bindGroupLayouts = {}
        };

        Handle<PipelineLayout_t> pipelineLayoutHandle;

        SUBCASE("Going Out Of Scope")
        {
            {
                // WHEN
                PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);
                pipelineLayoutHandle = pipelineLayout.handle();

                // THEN
                CHECK(pipelineLayout.isValid());
                CHECK(pipelineLayoutHandle.isValid());
                CHECK(api->resourceManager()->getPipelineLayout(pipelineLayoutHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getPipelineLayout(pipelineLayoutHandle) == nullptr);
        }

        SUBCASE("Move assignment")
        {
            // WHEN
            PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);
            pipelineLayoutHandle = pipelineLayout.handle();

            // THEN
            CHECK(pipelineLayout.isValid());
            CHECK(pipelineLayoutHandle.isValid());
            CHECK(api->resourceManager()->getPipelineLayout(pipelineLayoutHandle) != nullptr);

            // WHEN
            pipelineLayout = {};

            // THEN
            CHECK(api->resourceManager()->getPipelineLayout(pipelineLayoutHandle) == nullptr);
        }
    }

    TEST_CASE("Comparison")
    {
        SUBCASE("Compare default constructed PipelineLayouts")
        {
            // GIVEN
            PipelineLayout a;
            PipelineLayout b;

            // THEN
            CHECK(a == b);
        }

        SUBCASE("Compare device created PipelineLayouts")
        {
            // GIVEN
            PipelineLayoutOptions pipelineLayoutOptions = {
                .bindGroupLayouts = {}
            };

            // WHEN
            PipelineLayout a = device.createPipelineLayout(pipelineLayoutOptions);
            PipelineLayout b = device.createPipelineLayout(pipelineLayoutOptions);

            // THEN
            CHECK(a != b);
            CHECK(a == a);
        }
    }
}

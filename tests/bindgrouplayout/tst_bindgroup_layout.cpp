/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/bind_group.h>
#include <KDGpu/bind_group_options.h>
#include <KDGpu/bind_group_layout.h>
#include <KDGpu/bind_group_layout_options.h>
#include <KDGpu/bind_group_description.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/buffer.h>
#include <KDGpu/device.h>
#include <KDGpu/sampler.h>
#include <KDGpu/instance.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDGpu;

TEST_SUITE("BindGroupLayout")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "BindGroupLayout",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);
    Device device = discreteGPUAdapter->createDevice(DeviceOptions{
            .requestedFeatures = discreteGPUAdapter->features(),
    });

    TEST_CASE("Construction")
    {
        SUBCASE("A default constructed BindGroupLayout is invalid")
        {
            // GIVEN
            BindGroupLayout t;
            // THEN
            REQUIRE(!t.isValid());
        }

        SUBCASE("A constructed BindGroupLayout from a Vulkan API")
        {
            // GIVEN
            const BindGroupLayoutOptions bindGroupLayoutOptions = {
                .bindings = { { // Camera uniforms
                                .binding = 0,
                                .count = 1,
                                .resourceType = ResourceBindingType::UniformBuffer,
                                .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit) } }
            };

            const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);

            // THEN
            CHECK(bindGroupLayout.isValid());
        }
    }

    TEST_CASE("Destruction")
    {
        // GIVEN
        const BindGroupLayoutOptions bindGroupLayoutOptions = {
            .bindings = { { // Camera uniforms
                            .binding = 0,
                            .count = 1,
                            .resourceType = ResourceBindingType::UniformBuffer,
                            .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit) } }
        };

        Handle<BindGroupLayout_t> bindGroupLayoutHandle;

        SUBCASE("Going Out Of Scope")
        {
            {
                // WHEN
                const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);
                bindGroupLayoutHandle = bindGroupLayout.handle();

                // THEN
                CHECK(bindGroupLayout.isValid());
                CHECK(bindGroupLayoutHandle.isValid());
                CHECK(api->resourceManager()->getBindGroupLayout(bindGroupLayoutHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getBindGroupLayout(bindGroupLayoutHandle) == nullptr);
        }

        SUBCASE("Move assignment")
        {
            // WHEN
            BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);
            bindGroupLayoutHandle = bindGroupLayout.handle();

            // THEN
            CHECK(bindGroupLayout.isValid());
            CHECK(bindGroupLayoutHandle.isValid());
            CHECK(api->resourceManager()->getBindGroupLayout(bindGroupLayoutHandle) != nullptr);

            // WHEN
            bindGroupLayout = {};

            // THEN
            CHECK(api->resourceManager()->getBindGroupLayout(bindGroupLayoutHandle) == nullptr);
        }
    }

    TEST_CASE("Dynamic Indexing")
    {
        SUBCASE("VariableBindGroupEntriesCount")
        {
            if (!discreteGPUAdapter->features().shaderUniformBufferArrayNonUniformIndexing ||
                !discreteGPUAdapter->features().bindGroupBindingVariableDescriptorCount ||
                !discreteGPUAdapter->features().runtimeBindGroupArray)
                return;

            // GIVEN
            const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(BindGroupLayoutOptions{
                    .bindings = {
                            {
                                    .binding = 0,
                                    .count = 4,
                                    .resourceType = ResourceBindingType::UniformBuffer,
                                    .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit),
                                    .flags = { ResourceBindingFlagBits::VariableBindGroupEntriesCountBit },
                            },
                    },
            });

            // THEN
            CHECK(bindGroupLayout.isValid());
        }
    }

    TEST_CASE("Immutable Sampler")
    {
        SUBCASE("Immutable Sampler on Sampler Binding")
        {
            // GIVEN
            Sampler s = device.createSampler(SamplerOptions{});

            // THEN
            CHECK(s.isValid());

            // WHEN

            const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(BindGroupLayoutOptions{
                    .bindings = {
                            {
                                    .binding = 0,
                                    .count = 1,
                                    .resourceType = ResourceBindingType::Sampler,
                                    .shaderStages = ShaderStageFlags(ShaderStageFlagBits::FragmentBit),
                                    .immutableSamplers = { s.handle() },
                            },
                    },
            });

            // THEN
            CHECK(bindGroupLayout.isValid());
        }

        SUBCASE("Multiple Immutable Sampler on Sampler Binding")
        {
            // GIVEN
            Sampler s = device.createSampler(SamplerOptions{});

            // THEN
            CHECK(s.isValid());

            // WHEN

            const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(BindGroupLayoutOptions{
                    .bindings = {
                            {
                                    .binding = 0,
                                    .count = 4,
                                    .resourceType = ResourceBindingType::Sampler,
                                    .shaderStages = ShaderStageFlags(ShaderStageFlagBits::FragmentBit),
                                    .immutableSamplers = { s.handle(), s.handle(), s.handle(), s.handle() },
                            },
                    },
            });

            // THEN
            CHECK(bindGroupLayout.isValid());
        }

        SUBCASE("Immutable Sampler on CombinedImageSampler Binding")
        {
            // GIVEN
            Sampler s = device.createSampler(SamplerOptions{});

            // THEN
            CHECK(s.isValid());

            // WHEN

            const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(BindGroupLayoutOptions{
                    .bindings = {
                            {
                                    .binding = 0,
                                    .count = 1,
                                    .resourceType = ResourceBindingType::CombinedImageSampler,
                                    .shaderStages = ShaderStageFlags(ShaderStageFlagBits::FragmentBit),
                                    .immutableSamplers = { s.handle() },
                            },
                    },
            });

            // THEN
            CHECK(bindGroupLayout.isValid());
        }
    }

#if defined(VK_KHR_push_descriptor)
    TEST_CASE("Push BindGroup")
    {
        // GIVEN
        const BindGroupLayoutOptions bindGroupLayoutOptions = {
            .bindings = {
                    { .binding = 0,
                      .count = 1,
                      .resourceType = ResourceBindingType::UniformBuffer,
                      .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit) },
            },
            .flags = BindGroupLayoutFlagBits::PushBindGroup,
        };

        // THEN
        CHECK(discreteGPUAdapter->properties().pushBindGroupProperties.maxPushBindGroups > 0);

        // WHEN
        BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);

        // THEN
        CHECK(bindGroupLayout.isValid());
    }
#endif

    TEST_CASE("Comparison")
    {
        SUBCASE("Compare default constructed BindGroupLayouts")
        {
            // GIVEN
            BindGroupLayout a;
            BindGroupLayout b;

            // THEN
            CHECK(a == b);
        }

        SUBCASE("Compare device created compatible BindGroupLayouts")
        {
            // GIVEN
            const BindGroupLayoutOptions bindGroupLayoutOptions = {
                .bindings = { { // Camera uniforms
                                .binding = 0,
                                .count = 1,
                                .resourceType = ResourceBindingType::UniformBuffer,
                                .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit) } }
            };

            // WHEN
            BindGroupLayout a = device.createBindGroupLayout(bindGroupLayoutOptions);
            BindGroupLayout b = device.createBindGroupLayout(bindGroupLayoutOptions);

            // THEN
            CHECK(a.isCompatibleWith(b.handle()));
            CHECK(a == b);
        }

        SUBCASE("Compare incompatible BindGroupLayouts")
        {
            // GIVEN
            const BindGroupLayoutOptions bindGroupLayoutOptions1 = {
                .bindings = { { // Camera uniforms
                                .binding = 0,
                                .count = 1,
                                .resourceType = ResourceBindingType::UniformBuffer,
                                .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit) } }
            };
            const BindGroupLayoutOptions bindGroupLayoutOptions2 = {
                .bindings = { { .binding = 0,
                                .count = 1,
                                .resourceType = ResourceBindingType::UniformBuffer,
                                .shaderStages = ShaderStageFlags(ShaderStageFlagBits::FragmentBit) } }
            };

            // WHEN
            BindGroupLayout a = device.createBindGroupLayout(bindGroupLayoutOptions1);
            BindGroupLayout b = device.createBindGroupLayout(bindGroupLayoutOptions2);

            // THEN
            CHECK(!a.isCompatibleWith(b.handle()));
            CHECK(a != b);
        }
    }
}

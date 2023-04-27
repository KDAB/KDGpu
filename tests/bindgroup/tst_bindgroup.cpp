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
#include <KDGpu/instance.h>
#include <KDGpu/texture_options.h>
#include <KDGpu/sampler_options.h>
#include <KDGpu/texture_view.h>
#include <KDGpu/sampler.h>
#include <KDGpu/texture.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDGpu;

TEST_SUITE("BindGroup")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "BindGroup",
            .applicationVersion = SERENITY_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);
    Device device = discreteGPUAdapter->createDevice();

    TEST_CASE("Construction")
    {
        SUBCASE("A default constructed BindGroup is invalid")
        {
            // GIVEN
            BindGroup t;
            // THEN
            REQUIRE(!t.isValid());
        }

        SUBCASE("A constructed BindGroup from a Vulkan API")
        {
            // GIVEN
            BufferOptions uboOptions = {
                .size = 16 * sizeof(float),
                .usage = BufferUsageFlagBits::UniformBufferBit,
                .memoryUsage = MemoryUsage::CpuToGpu
            };
            auto ubo = device.createBuffer(uboOptions);

            const BindGroupLayoutOptions bindGroupLayoutOptions = {
                .bindings = { { .binding = 0,
                                .count = 1,
                                .resourceType = ResourceBindingType::UniformBuffer,
                                .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit) } }
            };

            const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);

            const BindGroupOptions bindGroupOptions = {
                .layout = bindGroupLayout,
                .resources = {
                        { .binding = 0,
                          .resource = UniformBufferBinding{ .buffer = ubo } },
                }
            };

            // WHEN
            BindGroup t = device.createBindGroup(bindGroupOptions);

            // THEN
            CHECK(t.isValid());
        }
    }

    TEST_CASE("Update BindGroup")
    {
        SUBCASE("UBO")
        {
            // GIVEN
            BufferOptions uboOptions = {
                .size = 16 * sizeof(float),
                .usage = BufferUsageFlagBits::UniformBufferBit,
                .memoryUsage = MemoryUsage::CpuToGpu
            };
            auto ubo = device.createBuffer(uboOptions);

            const BindGroupLayoutOptions bindGroupLayoutOptions = {
                .bindings = { { // Camera uniforms
                                .binding = 0,
                                .count = 1,
                                .resourceType = ResourceBindingType::UniformBuffer,
                                .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit) } }
            };

            const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);

            const BindGroupOptions bindGroupOptions = {
                .layout = bindGroupLayout,
                .resources = {
                        { .binding = 0,
                          .resource = UniformBufferBinding{ .buffer = ubo } },
                }
            };

            // WHEN
            BindGroup t = device.createBindGroup(bindGroupOptions);

            // THEN
            CHECK(t.isValid());

            // WHEN
            t.update(BindGroupEntry{ .binding = 0, .resource = UniformBufferBinding{ .buffer = ubo } });
        }

        SUBCASE("TextureView")
        {
            // GIVEN
            const TextureOptions textureOptions = {
                .type = TextureType::TextureType2D,
                .format = Format::R8G8B8A8_SNORM,
                .extent = { 512, 512, 1 },
                .mipLevels = 1,
                .usage = TextureUsageFlagBits::SampledBit | TextureUsageFlagBits::ColorAttachmentBit,
                .memoryUsage = MemoryUsage::GpuOnly
            };

            const SamplerOptions samplerOptions = {};

            const TextureViewOptions tvOptions = {
                .viewType = ViewType::ViewType2D,
                .format = Format::R8G8B8A8_SNORM
            };

            // WHEN
            Texture t = device.createTexture(textureOptions);
            TextureView tv = t.createView(tvOptions);
            Sampler s = device.createSampler(samplerOptions);

            // THEN
            CHECK(t.isValid());
            CHECK(tv.isValid());
            CHECK(s.isValid());

            const BindGroupLayoutOptions bindGroupLayoutOptions = {
                .bindings = { { .binding = 0,
                                .count = 1,
                                .resourceType = ResourceBindingType::CombinedImageSampler,
                                .shaderStages = ShaderStageFlags(ShaderStageFlagBits::FragmentBit) } }
            };

            const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);

            const BindGroupOptions bindGroupOptions = {
                .layout = bindGroupLayout,
                .resources = {
                        { .binding = 0,
                          .resource = TextureViewBinding{ .textureView = tv, .sampler = s } },
                }
            };

            // WHEN
            BindGroup b = device.createBindGroup(bindGroupOptions);

            // THEN
            CHECK(b.isValid());

            // WHEN
            b.update(BindGroupEntry{ .binding = 0, .resource = TextureViewBinding{ .textureView = tv, .sampler = s } });
        }

        SUBCASE("Dynamic UBO")
        {
            // GIVEN
            BufferOptions uboOptions = {
                .size = 16 * sizeof(float),
                .usage = BufferUsageFlagBits::UniformBufferBit,
                .memoryUsage = MemoryUsage::CpuToGpu
            };
            auto ubo = device.createBuffer(uboOptions);

            const BindGroupLayoutOptions bindGroupLayoutOptions = {
                .bindings = { { // Camera uniforms
                                .binding = 0,
                                .count = 1,
                                .resourceType = ResourceBindingType::DynamicUniformBuffer,
                                .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit) } }
            };

            const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);

            const BindGroupOptions bindGroupOptions = {
                .layout = bindGroupLayout,
                .resources = {
                        { .binding = 0,
                          .resource = DynamicUniformBufferBinding{ .buffer = ubo } },
                }
            };

            // WHEN
            BindGroup t = device.createBindGroup(bindGroupOptions);

            // THEN
            CHECK(t.isValid());

            // WHEN
            t.update(BindGroupEntry{ .binding = 0, .resource = DynamicUniformBufferBinding{ .buffer = ubo } });
        }
    }

    TEST_CASE("Destruction")
    {
        // GIVEN
        BufferOptions uboOptions = {
            .size = 16 * sizeof(float),
            .usage = BufferUsageFlagBits::UniformBufferBit,
            .memoryUsage = MemoryUsage::CpuToGpu
        };
        auto ubo = device.createBuffer(uboOptions);

        const BindGroupLayoutOptions bindGroupLayoutOptions = {
            .bindings = { { // Camera uniforms
                            .binding = 0,
                            .count = 1,
                            .resourceType = ResourceBindingType::UniformBuffer,
                            .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit) } }
        };

        const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);

        const BindGroupOptions bindGroupOptions = {
            .layout = bindGroupLayout,
            .resources = {
                    { .binding = 0,
                      .resource = UniformBufferBinding{ .buffer = ubo } },
            }
        };

        Handle<BindGroup_t> bindGroupHandle;

        SUBCASE("Going Out Of Scope")
        {
            {
                // WHEN
                BindGroup bindGroup = device.createBindGroup(bindGroupOptions);
                bindGroupHandle = bindGroup.handle();

                // THEN
                CHECK(bindGroup.isValid());
                CHECK(bindGroup.isValid());
                CHECK(api->resourceManager()->getBindGroup(bindGroupHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getBindGroup(bindGroupHandle) == nullptr);
        }

        SUBCASE("Move assignment")
        {
            // WHEN
            BindGroup bindGroup = device.createBindGroup(bindGroupOptions);
            bindGroupHandle = bindGroup.handle();

            // THEN
            CHECK(bindGroup.isValid());
            CHECK(bindGroup.isValid());
            CHECK(api->resourceManager()->getBindGroup(bindGroupHandle) != nullptr);

            // WHEN
            bindGroup = {};

            // THEN
            CHECK(api->resourceManager()->getBindGroup(bindGroupHandle) == nullptr);
        }
    }

    TEST_CASE("Comparison")
    {
        SUBCASE("Compare default constructed BindGroups")
        {
            // GIVEN
            BindGroup a;
            BindGroup b;

            // THEN
            CHECK(a == b);
        }

        SUBCASE("Compare device created BindGroups")
        {
            // GIVEN
            BufferOptions uboOptions = {
                .size = 16 * sizeof(float),
                .usage = BufferUsageFlagBits::UniformBufferBit,
                .memoryUsage = MemoryUsage::CpuToGpu
            };
            auto ubo = device.createBuffer(uboOptions);

            const BindGroupLayoutOptions bindGroupLayoutOptions = {
                .bindings = { { // Camera uniforms
                                .binding = 0,
                                .count = 1,
                                .resourceType = ResourceBindingType::UniformBuffer,
                                .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit) } }
            };

            const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);

            const BindGroupOptions bindGroupOptions = {
                .layout = bindGroupLayout,
                .resources = {
                        { .binding = 0,
                          .resource = UniformBufferBinding{ .buffer = ubo } },
                }
            };

            // WHEN
            BindGroup a = device.createBindGroup(bindGroupOptions);
            BindGroup b = device.createBindGroup(bindGroupOptions);

            // THEN
            CHECK(a != b);
        }
    }
}

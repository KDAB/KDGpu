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
#include <KDGpu/bind_group_pool.h>
#include <KDGpu/bind_group_pool_options.h>
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
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);
    Device device = discreteGPUAdapter->createDevice(DeviceOptions{
            .requestedFeatures = discreteGPUAdapter->features(),
    });

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

        SUBCASE("SSBO")
        {
            // GIVEN
            BufferOptions uboOptions = {
                .size = 16 * sizeof(float),
                .usage = BufferUsageFlagBits::StorageBufferBit,
                .memoryUsage = MemoryUsage::GpuOnly
            };
            auto ssbo = device.createBuffer(uboOptions);

            const BindGroupLayoutOptions bindGroupLayoutOptions = {
                .bindings = { { // Camera uniforms
                                .binding = 0,
                                .count = 1,
                                .resourceType = ResourceBindingType::StorageBuffer,
                                .shaderStages = ShaderStageFlags(ShaderStageFlagBits::ComputeBit) } }
            };

            const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);

            const BindGroupOptions bindGroupOptions = {
                .layout = bindGroupLayout,
                .resources = {
                        { .binding = 0,
                          .resource = StorageBufferBinding{ .buffer = ssbo } },
                }
            };

            // WHEN
            BindGroup t = device.createBindGroup(bindGroupOptions);

            // THEN
            CHECK(t.isValid());

            // WHEN
            t.update(BindGroupEntry{ .binding = 0, .resource = StorageBufferBinding{ .buffer = ssbo } });
        }

        SUBCASE("TextureViewSampler")
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
                .format = Format::R8G8B8A8_SNORM,
                .range = {
                        .aspectMask = TextureAspectFlagBits::ColorBit,
                },
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
                          .resource = TextureViewSamplerBinding{ .textureView = tv, .sampler = s } },
                }
            };

            // WHEN
            BindGroup b = device.createBindGroup(bindGroupOptions);

            // THEN
            CHECK(b.isValid());

            // WHEN
            b.update(BindGroupEntry{ .binding = 0, .resource = TextureViewSamplerBinding{ .textureView = tv, .sampler = s } });
        }

        SUBCASE("TextureViewSampler Immutable Sampler")
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
                .format = Format::R8G8B8A8_SNORM,
                .range = {
                        .aspectMask = TextureAspectFlagBits::ColorBit,
                },
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
                .bindings = {
                        {
                                .binding = 0,
                                .count = 1,
                                .resourceType = ResourceBindingType::CombinedImageSampler,
                                .shaderStages = ShaderStageFlags(ShaderStageFlagBits::FragmentBit),
                                .immutableSamplers = { s.handle() },
                        },
                },
            };

            const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);

            const BindGroupOptions bindGroupOptions = {
                .layout = bindGroupLayout,
                .resources = {
                        { .binding = 0,
                          .resource = TextureViewSamplerBinding{ .textureView = tv } }, // No Sampler since we've set an immutable sampler on the BindGroupLayout
                }
            };

            // WHEN
            BindGroup b = device.createBindGroup(bindGroupOptions);

            // THEN
            CHECK(b.isValid());

            // WHEN
            b.update(BindGroupEntry{ .binding = 0, .resource = TextureViewSamplerBinding{ .textureView = tv } });
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

            const TextureViewOptions tvOptions = {
                .viewType = ViewType::ViewType2D,
                .format = Format::R8G8B8A8_SNORM,
                .range = {
                        .aspectMask = TextureAspectFlagBits::ColorBit,
                },
            };

            // WHEN
            Texture t = device.createTexture(textureOptions);
            TextureView tv = t.createView(tvOptions);

            // THEN
            CHECK(t.isValid());
            CHECK(tv.isValid());

            const BindGroupLayoutOptions bindGroupLayoutOptions = {
                .bindings = { { .binding = 0,
                                .count = 1,
                                .resourceType = ResourceBindingType::SampledImage,
                                .shaderStages = ShaderStageFlags(ShaderStageFlagBits::FragmentBit) } }
            };

            const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);

            const BindGroupOptions bindGroupOptions = {
                .layout = bindGroupLayout,
                .resources = {
                        { .binding = 0,
                          .resource = TextureViewBinding{ .textureView = tv } },
                }
            };

            // WHEN
            BindGroup b = device.createBindGroup(bindGroupOptions);

            // THEN
            CHECK(b.isValid());

            // WHEN
            b.update(BindGroupEntry{ .binding = 0, .resource = TextureViewBinding{ .textureView = tv } });
        }

        SUBCASE("InputAttachmentBinding")
        {
            // GIVEN
            const TextureOptions textureOptions = {
                .type = TextureType::TextureType2D,
                .format = Format::R8G8B8A8_SNORM,
                .extent = { 512, 512, 1 },
                .mipLevels = 1,
                .usage = TextureUsageFlagBits::SampledBit | TextureUsageFlagBits::ColorAttachmentBit | TextureUsageFlagBits::InputAttachmentBit,
                .memoryUsage = MemoryUsage::GpuOnly
            };

            const TextureViewOptions tvOptions = {
                .viewType = ViewType::ViewType2D,
                .format = Format::R8G8B8A8_SNORM,
                .range = {
                        .aspectMask = TextureAspectFlagBits::ColorBit,
                },
            };

            // WHEN
            Texture t = device.createTexture(textureOptions);
            TextureView tv = t.createView(tvOptions);

            // THEN
            CHECK(t.isValid());
            CHECK(tv.isValid());

            const BindGroupLayoutOptions bindGroupLayoutOptions = {
                .bindings = { { .binding = 0,
                                .count = 1,
                                .resourceType = ResourceBindingType::InputAttachment,
                                .shaderStages = ShaderStageFlags(ShaderStageFlagBits::FragmentBit) } }
            };

            const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);

            const BindGroupOptions bindGroupOptions = {
                .layout = bindGroupLayout,
                .resources = {
                        { .binding = 0,
                          .resource = InputAttachmentBinding{ .textureView = tv } },
                }
            };

            // WHEN
            BindGroup b = device.createBindGroup(bindGroupOptions);

            // THEN
            CHECK(b.isValid());

            // WHEN
            b.update(BindGroupEntry{ .binding = 0, .resource = InputAttachmentBinding{ .textureView = tv } });
        }

        SUBCASE("Sampler")
        {
            // GIVEN
            const SamplerOptions samplerOptions = {};

            // WHEN
            Sampler s = device.createSampler(samplerOptions);

            // THEN
            CHECK(s.isValid());

            const BindGroupLayoutOptions bindGroupLayoutOptions = {
                .bindings = { { .binding = 0,
                                .count = 1,
                                .resourceType = ResourceBindingType::Sampler,
                                .shaderStages = ShaderStageFlags(ShaderStageFlagBits::FragmentBit) } }
            };

            const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);

            const BindGroupOptions bindGroupOptions = {
                .layout = bindGroupLayout,
                .resources = {
                        { .binding = 0,
                          .resource = SamplerBinding{ .sampler = s } },
                }
            };

            // WHEN
            BindGroup b = device.createBindGroup(bindGroupOptions);

            // THEN
            CHECK(b.isValid());

            // WHEN
            b.update(BindGroupEntry{ .binding = 0, .resource = SamplerBinding{ .sampler = s } });
        }

        SUBCASE("Image")
        {
            // GIVEN
            const TextureOptions textureOptions = {
                .type = TextureType::TextureType2D,
                .format = Format::R8G8B8A8_SNORM,
                .extent = { 512, 512, 1 },
                .mipLevels = 1,
                .usage = TextureUsageFlagBits::StorageBit,
                .memoryUsage = MemoryUsage::GpuOnly
            };

            const TextureViewOptions tvOptions = {
                .viewType = ViewType::ViewType2D,
                .format = Format::R8G8B8A8_SNORM,
                .range = {
                        .aspectMask = TextureAspectFlagBits::ColorBit,
                },
            };

            // WHEN
            Texture t = device.createTexture(textureOptions);
            TextureView tv = t.createView(tvOptions);

            // THEN
            CHECK(t.isValid());
            CHECK(tv.isValid());

            const BindGroupLayoutOptions bindGroupLayoutOptions = {
                .bindings = { { .binding = 0,
                                .count = 1,
                                .resourceType = ResourceBindingType::StorageImage,
                                .shaderStages = ShaderStageFlags(ShaderStageFlagBits::FragmentBit) } }
            };

            const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);

            const BindGroupOptions bindGroupOptions = {
                .layout = bindGroupLayout,
                .resources = {
                        { .binding = 0,
                          .resource = ImageBinding{ .textureView = tv } },
                }
            };

            // WHEN
            BindGroup b = device.createBindGroup(bindGroupOptions);

            // THEN
            CHECK(b.isValid());

            // WHEN
            b.update(BindGroupEntry{ .binding = 0, .resource = ImageBinding{ .textureView = tv } });
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

    TEST_CASE("Dynamic BindGroup Indexing")
    {
        SUBCASE("VariableBindGroupEntriesCount")
        {
            if (!discreteGPUAdapter->features().shaderUniformBufferArrayNonUniformIndexing ||
                !discreteGPUAdapter->features().runtimeBindGroupArray)
                return;

            // GIVEN
            const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(BindGroupLayoutOptions{
                    .bindings = {
                            {
                                    // Array of 4 UBOs on Binding 0
                                    .binding = 0,
                                    .count = 4,
                                    .resourceType = ResourceBindingType::UniformBuffer,
                                    .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit),
                                    // Means that as far as the shader is concerned, it has no idea how many UBOs are in the array
                                    .flags = { ResourceBindingFlagBits::VariableBindGroupEntriesCountBit },
                            },
                    },
            });

            // THEN
            CHECK(bindGroupLayout.isValid());

            // WHEN
            BindGroup bindGroup = device.createBindGroup(BindGroupOptions{
                    .layout = bindGroupLayout,
                    .maxVariableArrayLength = 4,
            });

            // THEN
            CHECK(bindGroup.isValid());

            // WHEN
            const BufferOptions uboOptions = {
                .size = 16 * sizeof(float),
                .usage = BufferUsageFlagBits::UniformBufferBit,
                .memoryUsage = MemoryUsage::CpuToGpu
            };
            const Buffer ubo1 = device.createBuffer(uboOptions);
            const Buffer ubo2 = device.createBuffer(uboOptions);
            const Buffer ubo3 = device.createBuffer(uboOptions);
            const Buffer ubo4 = device.createBuffer(uboOptions);

            CHECK(ubo1.isValid());
            CHECK(ubo2.isValid());
            CHECK(ubo3.isValid());
            CHECK(ubo4.isValid());

            bindGroup.update(BindGroupEntry{ .binding = 0, .resource = UniformBufferBinding{ .buffer = ubo1 }, .arrayElement = 0 });
            bindGroup.update(BindGroupEntry{ .binding = 0, .resource = UniformBufferBinding{ .buffer = ubo2 }, .arrayElement = 1 });
            bindGroup.update(BindGroupEntry{ .binding = 0, .resource = UniformBufferBinding{ .buffer = ubo3 }, .arrayElement = 2 });
            bindGroup.update(BindGroupEntry{ .binding = 0, .resource = UniformBufferBinding{ .buffer = ubo4 }, .arrayElement = 3 });

            // THEN -> No validation error
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

    TEST_CASE("BindGroup with dedicated BindGroupPool")
    {
        SUBCASE("Create BindGroup using a dedicated BindGroupPool")
        {
            // GIVEN - Create a dedicated BindGroupPool with specific configuration
            const BindGroupPoolOptions poolOptions{
                .label = "Dedicated Test Pool",
                .uniformBufferCount = 10,
                .dynamicUniformBufferCount = 2,
                .storageBufferCount = 5,
                .textureSamplerCount = 8,
                .textureCount = 8,
                .samplerCount = 4,
                .imageCount = 2,
                .inputAttachmentCount = 1,
                .maxBindGroupCount = 20,
                .flags = BindGroupPoolFlagBits::CreateFreeBindGroups
            };

            BindGroupPool pool = device.createBindGroupPool(poolOptions);

            // THEN - Pool should be valid
            CHECK(pool.isValid());

            // WHEN
            Buffer ubo = device.createBuffer(BufferOptions{
                    .size = 16 * sizeof(float),
                    .usage = BufferUsageFlagBits::UniformBufferBit,
                    .memoryUsage = MemoryUsage::CpuToGpu,
            });

            const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(BindGroupLayoutOptions{
                    .bindings = {
                            { .binding = 0,
                              .count = 1,
                              .resourceType = ResourceBindingType::UniformBuffer,
                              .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit) },
                    },
            });

            const BindGroupOptions bindGroupOptions = {
                .layout = bindGroupLayout,
                .resources = {
                        { .binding = 0,
                          .resource = UniformBufferBinding{ .buffer = ubo } },
                },
                .bindGroupPool = pool // Use the dedicated pool
            };

            BindGroup bindGroup = device.createBindGroup(bindGroupOptions);

            // THEN - BindGroup should be valid and created from the dedicated pool
            CHECK(bindGroup.isValid());
        }

        SUBCASE("Create multiple BindGroups from the same pool")
        {
            // GIVEN - Create a BindGroupPool that can handle multiple bind groups
            const BindGroupPoolOptions poolOptions = {
                .label = "Multi BindGroup Pool",
                .uniformBufferCount = 20,
                .maxBindGroupCount = 5,
                .flags = BindGroupPoolFlagBits::CreateFreeBindGroups
            };

            BindGroupPool pool = device.createBindGroupPool(poolOptions);

            // THEN
            CHECK(pool.isValid());

            // Create resources for bind groups
            Buffer ubo = device.createBuffer(BufferOptions{
                    .size = 16 * sizeof(float),
                    .usage = BufferUsageFlagBits::UniformBufferBit,
                    .memoryUsage = MemoryUsage::CpuToGpu,
            });

            const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(BindGroupLayoutOptions{
                    .bindings = {
                            {
                                    .binding = 0,
                                    .count = 1,
                                    .resourceType = ResourceBindingType::UniformBuffer,
                                    .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit),
                            },
                    },
            });

            // WHEN - Create multiple BindGroups from the same pool
            const BindGroupOptions bindGroupOptions = {
                .layout = bindGroupLayout,
                .resources = {
                        { .binding = 0, .resource = UniformBufferBinding{ .buffer = ubo } },
                },
                .bindGroupPool = pool
            };

            BindGroup bindGroup1 = device.createBindGroup(bindGroupOptions);
            BindGroup bindGroup2 = device.createBindGroup(bindGroupOptions);

            // THEN - Both BindGroups should be valid
            CHECK(bindGroup1.isValid());
            CHECK(bindGroup2.isValid());
            CHECK(bindGroup1 != bindGroup2);
        }

        SUBCASE("Pool exhaustion behavior")
        {
            // GIVEN - Create a BindGroupPool with limited capacity
            const BindGroupPoolOptions poolOptions = {
                .label = "Limited Pool",
                .uniformBufferCount = 2, // Very limited
                .maxBindGroupCount = 2, // Only 2 bind groups max
                .flags = BindGroupPoolFlagBits::CreateFreeBindGroups
            };

            BindGroupPool pool = device.createBindGroupPool(poolOptions);
            CHECK(pool.isValid());

            // Create resources
            Buffer ubo = device.createBuffer(BufferOptions{
                    .size = 16 * sizeof(float),
                    .usage = BufferUsageFlagBits::UniformBufferBit,
                    .memoryUsage = MemoryUsage::CpuToGpu,
            });

            const BindGroupLayoutOptions bindGroupLayoutOptions = {
                .bindings = {
                        { .binding = 0,
                          .count = 1,
                          .resourceType = ResourceBindingType::UniformBuffer,
                          .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit) },
                },
            };

            const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);

            // WHEN - Create bind groups up to the pool limit
            const BindGroupOptions bindGroupOptions = {
                .layout = bindGroupLayout,
                .resources = {
                        { .binding = 0, .resource = UniformBufferBinding{ .buffer = ubo } },
                },
                .bindGroupPool = pool
            };

            BindGroup bindGroup1 = device.createBindGroup(bindGroupOptions);
            BindGroup bindGroup2 = device.createBindGroup(bindGroupOptions);

            // THEN - First two should succeed
            CHECK(bindGroup1.isValid());
            CHECK(bindGroup2.isValid());

            // WHEN - Trying to allocate more BindGroups than the pool can handle
            BindGroup bindGroup3 = device.createBindGroup(bindGroupOptions);

            // THEN -> failure due to pool being exhausted at this point
            CHECK(!bindGroup3.isValid());
        }

        SUBCASE("BindGroup become invalid after pool reset")
        {
            // GIVEN - Create a BindGroupPool with limited capacity
            const BindGroupPoolOptions poolOptions = {
                .label = "Limited Pool",
                .uniformBufferCount = 2, // Very limited
                .maxBindGroupCount = 2, // Only 2 bind groups max
                .flags = BindGroupPoolFlagBits::CreateFreeBindGroups
            };

            BindGroupPool pool = device.createBindGroupPool(poolOptions);
            CHECK(pool.isValid());

            // Create resources
            Buffer ubo = device.createBuffer(BufferOptions{
                    .size = 16 * sizeof(float),
                    .usage = BufferUsageFlagBits::UniformBufferBit,
                    .memoryUsage = MemoryUsage::CpuToGpu,
            });

            const BindGroupLayoutOptions bindGroupLayoutOptions = {
                .bindings = {
                        { .binding = 0,
                          .count = 1,
                          .resourceType = ResourceBindingType::UniformBuffer,
                          .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit) },
                },
            };

            const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);

            // WHEN - Create bind groups up to the pool limit
            const BindGroupOptions bindGroupOptions = {
                .layout = bindGroupLayout,
                .resources = {
                        { .binding = 0, .resource = UniformBufferBinding{ .buffer = ubo } },
                },
                .bindGroupPool = pool
            };

            BindGroup bindGroup = device.createBindGroup(bindGroupOptions);

            // THEN
            CHECK(bindGroup.isValid());

            // WHEN
            pool.reset();

            // THEN -> BindGroup is now invalid because underlying api resource has been reset
            CHECK(!bindGroup.isValid());
        }
    }
}

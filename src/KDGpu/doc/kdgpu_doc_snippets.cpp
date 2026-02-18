/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2026 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

/**
 * @file kdgpu_doc_snippets.cpp
 * @brief Code snippets referenced by KDGPU API documentation
 *
 * This file contains all code examples used in the KDGPU API documentation.
 * Each snippet is marked with tags that allow Doxygen to reference them using
 * the \snippet command.
 *
 * Note: This file is compiled as part of the documentation build to ensure
 * all code examples are valid and compilable.
 */

#include <KDGpu/buffer.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/texture.h>
#include <KDGpu/texture_options.h>
#include <KDGpu/texture_view.h>
#include <KDGpu/texture_view_options.h>
#include <KDGpu/device.h>
#include <KDGpu/instance.h>
#include <KDGpu/adapter.h>
#include <KDGpu/queue.h>
#include <KDGpu/bind_group.h>
#include <KDGpu/bind_group_options.h>
#include <KDGpu/bind_group_description.h>
#include <KDGpu/bind_group_layout.h>
#include <KDGpu/bind_group_layout_options.h>
#include <KDGpu/bind_group_pool.h>
#include <KDGpu/bind_group_pool_options.h>
#include <KDGpu/command_recorder.h>
#include <KDGpu/compute_pipeline.h>
#include <KDGpu/compute_pipeline_options.h>
#include <KDGpu/fence.h>
#include <KDGpu/gpu_semaphore.h>
#include <KDGpu/graphics_pipeline.h>
#include <KDGpu/graphics_pipeline_options.h>
#include <KDGpu/pipeline_layout.h>
#include <KDGpu/pipeline_layout_options.h>
#include <KDGpu/shader_module.h>
#include <KDGpu/swapchain.h>
#include <KDGpu/swapchain_options.h>
#include <KDGpu/surface.h>
#include <KDGpu/surface_options.h>
#include <KDGpu/sampler.h>
#include <KDGpu/sampler_options.h>
#include <KDGpu/render_pass_command_recorder_options.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

#include <vector>
#include <cstring>
#include <fstream>
#include <iostream>
#include <array>
#include <algorithm>
#include <cmath>

// GLM for math
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Placeholder implementations to make code compile
namespace {
struct Camera {
    auto viewMatrix() { return glm::mat4{}; }
    auto projectionMatrix() { return glm::mat4{}; }
};

//! [vertex_struct]
struct Vertex {
    float position[3];
    float color[3];
    float texCoord[2];
};
//! [vertex_struct]

//! [particle_struct]
struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
};
//! [particle_struct]

void *sourceData = nullptr;
size_t dataSize = 0;

//! [load_spirv]
std::vector<uint32_t> loadSpirv(const std::string &filepath)
{
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return {};
    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
    file.seekg(0);
    file.read(reinterpret_cast<char *>(buffer.data()), fileSize);
    return buffer;
}
//! [load_spirv]

std::vector<uint8_t> loadImageFile(const std::string &filepath)
{
    return std::vector<uint8_t>(1024 * 1024 * 4); // Placeholder
}
} // namespace

// =============================================================================
// Buffer Examples (buffer.h)
// =============================================================================

void buffer_examples(KDGpu::Device &device)
{
    //! [buffer_vertex_creation]
    struct Vertex {
        float position[3];
        float color[3];
    };

    std::vector<Vertex> vertices = { /* ... */ };

    KDGpu::Buffer vertexBuffer = device.createBuffer(KDGpu::BufferOptions{
                                                             .size = vertices.size() * sizeof(Vertex),
                                                             .usage = KDGpu::BufferUsageFlagBits::VertexBufferBit,
                                                             .memoryUsage = KDGpu::MemoryUsage::CpuToGpu, // CPU-writable, GPU-readable
                                                     },
                                                     vertices.data()); // Optional initial data
    //! [buffer_vertex_creation]

    //! [buffer_index_creation]
    std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

    KDGpu::Buffer indexBuffer = device.createBuffer(KDGpu::BufferOptions{
                                                            .size = indices.size() * sizeof(uint32_t),
                                                            .usage = KDGpu::BufferUsageFlagBits::IndexBufferBit,
                                                            .memoryUsage = KDGpu::MemoryUsage::CpuToGpu,
                                                    },
                                                    indices.data());
    //! [buffer_index_creation]

    //! [buffer_uniform_creation]
    struct CameraUniforms {
        glm::mat4 view;
        glm::mat4 projection;
    };

    KDGpu::Buffer uniformBuffer = device.createBuffer(KDGpu::BufferOptions{
            .size = sizeof(CameraUniforms),
            .usage = KDGpu::BufferUsageFlagBits::UniformBufferBit,
            .memoryUsage = KDGpu::MemoryUsage::CpuToGpu, // For frequent updates
    });

    // Update each frame
    Camera camera;
    void *mapped = uniformBuffer.map();
    CameraUniforms *uniforms = static_cast<CameraUniforms *>(mapped);
    uniforms->view = camera.viewMatrix();
    uniforms->projection = camera.projectionMatrix();
    uniformBuffer.flush(); // Ensure GPU sees changes
    uniformBuffer.unmap();
    //! [buffer_uniform_creation]

    //! [buffer_storage_creation]
    KDGpu::Buffer storageBuffer = device.createBuffer(KDGpu::BufferOptions{
            .size = 1024 * 1024, // 1 MB
            .usage = KDGpu::BufferUsageFlagBits::StorageBufferBit,
            .memoryUsage = KDGpu::MemoryUsage::GpuOnly, // GPU-only for best performance
    });
    //! [buffer_storage_creation]

    //! [buffer_memory_usage_patterns]
    // CPU to GPU: Writable from CPU, readable by GPU (staging, dynamic data)
    auto cpuToGpu = KDGpu::MemoryUsage::CpuToGpu;

    // GPU only: Best performance, require staging for uploads
    auto gpuOnly = KDGpu::MemoryUsage::GpuOnly;

    // GPU to CPU: For readback (screenshots, compute results)
    auto gpuToCpu = KDGpu::MemoryUsage::GpuToCpu;
    //! [buffer_memory_usage_patterns]

    //! [buffer_mapping_unmapping]
    // Map for writing
    void *data = storageBuffer.map();
    memcpy(data, sourceData, dataSize);
    storageBuffer.unmap();

    // For non-coherent memory, flush to make changes visible to GPU
    storageBuffer.flush();

    // For reading GPU results, invalidate before reading
    storageBuffer.invalidate();
    void *results = storageBuffer.map();
    // ... read results ...
    storageBuffer.unmap();
    //! [buffer_mapping_unmapping]

    //! [buffer_device_address]
    KDGpu::Buffer buffer2 = device.createBuffer(KDGpu::BufferOptions{
            .size = 1024,
            .usage = KDGpu::BufferUsageFlagBits::StorageBufferBit |
                    KDGpu::BufferUsageFlagBits::ShaderDeviceAddressBit,
            .memoryUsage = KDGpu::MemoryUsage::GpuOnly,
    });

    KDGpu::BufferDeviceAddress deviceAddress = buffer2.bufferDeviceAddress();
    // Pass deviceAddress to shader via push constants or uniforms
    //! [buffer_device_address]
}

// =============================================================================
// TextureView Examples (texture_view.h)
// =============================================================================

void textureview_examples(KDGpu::Device &device)
{
    KDGpu::Texture texture;
    KDGpu::BindGroupLayout layout;
    KDGpu::Sampler sampler;
    uint32_t width = 1024, height = 1024;
    KDGpu::TextureOptions textureOptions;

    //! [textureview_default]
    KDGpu::Texture texture1 = device.createTexture(textureOptions);
    KDGpu::TextureView view = texture1.createView(); // View entire texture

    // Use in bind group
    KDGpu::BindGroup bindGroupTex = device.createBindGroup(KDGpu::BindGroupOptions{
            .layout = layout.handle(),
            .resources = {
                    {
                            .binding = 0,
                            .resource = KDGpu::TextureViewSamplerBinding{
                                    .textureView = view.handle(),
                                    .sampler = sampler.handle(),
                            },
                    },
            },
    });
    //! [textureview_default]

    //! [textureview_mip_level]
    // Create view for mip level 2 only
    KDGpu::TextureView mipView = texture1.createView(KDGpu::TextureViewOptions{
            .range = {
                    .aspectMask = KDGpu::TextureAspectFlagBits::ColorBit,
                    .baseMipLevel = 2,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
            },
    });
    //! [textureview_mip_level]

    //! [textureview_cubemap_faces]
    KDGpu::Texture cubemap = device.createTexture(KDGpu::TextureOptions{
            .type = KDGpu::TextureType::TextureTypeCube,
            .format = KDGpu::Format::R8G8B8A8_SRGB,
            .extent = { 1024, 1024, 1 },
            .arrayLayers = 6,
    });

    // View as cube map (all 6 faces)
    KDGpu::TextureView cubeView = cubemap.createView(KDGpu::TextureViewOptions{
            .viewType = KDGpu::ViewType::ViewTypeCube,
    });

    // View individual faces as 2D textures
    std::array<KDGpu::TextureView, 6> faceViews;
    for (uint32_t face = 0; face < 6; ++face) {
        faceViews[face] = cubemap.createView(KDGpu::TextureViewOptions{
                .viewType = KDGpu::ViewType::ViewType2D,
                .range = {
                        .aspectMask = KDGpu::TextureAspectFlagBits::ColorBit,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = face,
                        .layerCount = 1,
                },
        });
    }
    //! [textureview_cubemap_faces]

    //! [textureview_depth_stencil]
    KDGpu::Texture depthStencil = device.createTexture(KDGpu::TextureOptions{
            .type = KDGpu::TextureType::TextureType2D,
            .format = KDGpu::Format::D24_UNORM_S8_UINT,
            .extent = { width, height, 1 },
            .usage = KDGpu::TextureUsageFlagBits::DepthStencilAttachmentBit,
    });

    // View depth and stencil together
    KDGpu::TextureView depthStencilView = depthStencil.createView(KDGpu::TextureViewOptions{
            .range = {
                    .aspectMask = KDGpu::TextureAspectFlagBits::DepthBit |
                            KDGpu::TextureAspectFlagBits::StencilBit,
            },
    });

    // View depth aspect only
    KDGpu::TextureView depthOnlyView = depthStencil.createView(KDGpu::TextureViewOptions{
            .range = { .aspectMask = KDGpu::TextureAspectFlagBits::DepthBit },
    });

    // View stencil aspect only
    KDGpu::TextureView stencilOnlyView = depthStencil.createView(KDGpu::TextureViewOptions{
            .range = { .aspectMask = KDGpu::TextureAspectFlagBits::StencilBit },
    });
    //! [textureview_depth_stencil]

    //! [textureview_array_texture]
    KDGpu::Texture textureArray = device.createTexture(KDGpu::TextureOptions{
            .type = KDGpu::TextureType::TextureType2D,
            .format = KDGpu::Format::R8G8B8A8_SRGB,
            .extent = { 512, 512, 1 },
            .arrayLayers = 10 // 10 layers
    });

    // View entire array
    KDGpu::TextureView arrayView = textureArray.createView(KDGpu::TextureViewOptions{
            .viewType = KDGpu::ViewType::ViewType2DArray,
            .range = {
                    .aspectMask = KDGpu::TextureAspectFlagBits::ColorBit,
                    .baseArrayLayer = 0,
                    .layerCount = 10,
            },
    });

    // View single layer
    KDGpu::TextureView singleLayer = textureArray.createView(KDGpu::TextureViewOptions{
            .viewType = KDGpu::ViewType::ViewType2D,
            .range = {
                    .aspectMask = KDGpu::TextureAspectFlagBits::ColorBit,
                    .baseArrayLayer = 3, // Layer 3
                    .layerCount = 1,
            },
    });
    //! [textureview_array_texture]

    KDGpu::CommandRecorder recorder = device.createCommandRecorder();
    KDGpu::TextureView colorTextureView, depthTextureView;

    //! [textureview_render_pass]
    auto renderPass = recorder.beginRenderPass(KDGpu::RenderPassCommandRecorderOptions{
            .colorAttachments = {
                    {
                            .view = colorTextureView, // TextureView, not Texture
                            .loadOperation = KDGpu::AttachmentLoadOperation::Clear,
                            .storeOperation = KDGpu::AttachmentStoreOperation::Store,
                            .clearValue = { 0.0f, 0.0f, 0.0f, 1.0f },
                    },
            },
            .depthStencilAttachment = {
                    .view = depthTextureView,
            },
    });
    //! [textureview_render_pass]

    //! [textureview_storage_image]
    KDGpu::Texture storageTexture = device.createTexture(KDGpu::TextureOptions{
            .type = KDGpu::TextureType::TextureType2D,
            .format = KDGpu::Format::R32G32B32A32_SFLOAT,
            .extent = { width, height, 1 },
            .usage = KDGpu::TextureUsageFlagBits::StorageBit,
    });

    KDGpu::TextureView storageView = storageTexture.createView();

    KDGpu::BindGroup bindGroupStorage = device.createBindGroup(KDGpu::BindGroupOptions{
            .layout = layout.handle(),
            .resources = {
                    {
                            .binding = 0,
                            .resource = KDGpu::ImageBinding{
                                    .textureView = storageView.handle(),
                            },
                    },
            },
    });
    //! [textureview_storage_image]
}

// =============================================================================
// Adapter Examples (adapter.h)
// =============================================================================

void adapter_examples(KDGpu::Instance &instance)
{
    //! [adapter_enumerate]
    std::vector<KDGpu::Adapter *> adapters = instance.adapters();
    for (KDGpu::Adapter *adapter : adapters) {
        KDGpu::AdapterProperties props = adapter->properties();
        std::cout << "GPU: " << props.deviceName << std::endl;
    }
    //! [adapter_enumerate]

    KDGpu::Adapter *adapter = adapters[0];

    //! [adapter_properties]
    KDGpu::AdapterProperties props = adapter->properties();
    std::cout << "Device: " << props.deviceName << std::endl;
    std::cout << "Vendor ID: 0x" << std::hex << props.vendorID << std::endl;
    std::cout << "Device ID: 0x" << std::hex << props.deviceID << std::endl;
    std::cout << "Driver version: " << props.driverVersion << std::endl;

    switch (props.deviceType) {
    case KDGpu::AdapterDeviceType::DiscreteGpu:
        std::cout << "Type: Discrete GPU" << std::endl;
        break;
    case KDGpu::AdapterDeviceType::IntegratedGpu:
        std::cout << "Type: Integrated GPU" << std::endl;
        break;
    case KDGpu::AdapterDeviceType::Cpu:
        std::cout << "Type: CPU/Software" << std::endl;
        break;
    default:
        break;
    }
    //! [adapter_properties]

    //! [adapter_features]
    const KDGpu::AdapterFeatures &features = adapter->features();
    if (features.geometryShader)
        std::cout << "Geometry shaders supported" << std::endl;
    if (features.tessellationShader)
        std::cout << "Tessellation supported" << std::endl;
    if (features.multiDrawIndirect)
        std::cout << "Multi-draw indirect supported" << std::endl;
    //! [adapter_features]

    //! [adapter_limits]
    const KDGpu::AdapterProperties &adapterProps = adapter->properties();
    std::cout << "Max texture size: " << adapterProps.limits.maxImageDimension2D << std::endl;
    std::cout << "Max uniform buffer range: " << adapterProps.limits.maxUniformBufferRange << std::endl;
    std::cout << "Max compute workgroup size: "
              << adapterProps.limits.maxComputeWorkGroupSize[0] << "x"
              << adapterProps.limits.maxComputeWorkGroupSize[1] << "x"
              << adapterProps.limits.maxComputeWorkGroupSize[2] << std::endl;
    //! [adapter_limits]

    //! [adapter_queue_types]
    auto queueTypes = adapter->queueTypes();
    for (size_t i = 0; i < queueTypes.size(); ++i) {
        std::cout << "Queue family " << i << ":" << std::endl;
        auto flags = queueTypes[i].flags;
        if (flags & KDGpu::QueueFlagBits::GraphicsBit)
            std::cout << "  - Graphics" << std::endl;
        if (flags & KDGpu::QueueFlagBits::ComputeBit)
            std::cout << "  - Compute" << std::endl;
        if (flags & KDGpu::QueueFlagBits::TransferBit)
            std::cout << "  - Transfer" << std::endl;
        std::cout << "  Queues available: " << queueTypes.size() << std::endl;
    }
    //! [adapter_queue_types]
}

// Helper function for adapter selection example
KDGpu::Adapter *selectDiscreteGpuHelper(KDGpu::Instance &instance)
{
    //! [adapter_select_discrete]
    for (KDGpu::Adapter *adapter : instance.adapters()) {
        if (adapter->properties().deviceType == KDGpu::AdapterDeviceType::DiscreteGpu)
            return adapter;
    }
    return instance.adapters().front(); // Fallback
    //! [adapter_select_discrete]
}

void adapter_examples_continued(KDGpu::Instance &instance)
{
    KDGpu::Adapter *adapter = instance.adapters()[0];

    //! [adapter_format_support]
    KDGpu::Format format = KDGpu::Format::R8G8B8A8_SRGB;
    KDGpu::FormatProperties formatProps = adapter->formatProperties(format);

    if (formatProps.optimalTilingFeatures & KDGpu::FormatFeatureFlagBit::SampledImageBit)
        std::cout << "Can use as sampled texture" << std::endl;
    if (formatProps.optimalTilingFeatures & KDGpu::FormatFeatureFlagBit::ColorAttachmentBit)
        std::cout << "Can use as render target" << std::endl;
    //! [adapter_format_support]

    KDGpu::Surface surface;

    //! [adapter_swapchain_info]
    auto swapchainProps = adapter->swapchainProperties(surface.handle());
    std::cout << "Supported present modes:" << std::endl;
    for (auto mode : swapchainProps.presentModes) {
        if (mode == KDGpu::PresentMode::Mailbox)
            std::cout << "  - Mailbox (low-latency triple buffering)" << std::endl;
        else if (mode == KDGpu::PresentMode::Immediate)
            std::cout << "  - Immediate (no vsync, tearing)" << std::endl;
        else if (mode == KDGpu::PresentMode::Fifo)
            std::cout << "  - FIFO (vsync)" << std::endl;
    }
    //! [adapter_swapchain_info]

    //! [adapter_create_device]
    KDGpu::Device device2 = adapter->createDevice(KDGpu::DeviceOptions{
            .requestedFeatures = {
                    .geometryShader = true,
                    .multiDrawIndirect = true,
            },
    });
    //! [adapter_create_device]
}

// =============================================================================
// BindGroupLayout Examples (bind_group_layout.h)
// =============================================================================

void bindgrouplayout_examples(KDGpu::Device &device)
{
    //! [bindgrouplayout_simple]
    KDGpu::BindGroupLayout layout = device.createBindGroupLayout(KDGpu::BindGroupLayoutOptions{
            .bindings = {
                    {
                            // Binding 0: uniform buffer
                            .binding = 0,
                            .resourceType = KDGpu::ResourceBindingType::UniformBuffer,
                            .shaderStages = KDGpu::ShaderStageFlags(KDGpu::ShaderStageFlagBits::VertexBit),
                    },
                    {
                            // Binding 1: combined image sampler
                            .binding = 1,
                            .resourceType = KDGpu::ResourceBindingType::CombinedImageSampler,
                            .shaderStages = KDGpu::ShaderStageFlags(KDGpu::ShaderStageFlagBits::FragmentBit),
                    },
            },
    });
    //! [bindgrouplayout_simple]

    //! [bindgrouplayout_compute]
    KDGpu::BindGroupLayout computeLayout = device.createBindGroupLayout(KDGpu::BindGroupLayoutOptions{
            .bindings = {
                    {
                            // Input storage buffer
                            .binding = 0,
                            .resourceType = KDGpu::ResourceBindingType::StorageBuffer,
                            .shaderStages = KDGpu::ShaderStageFlags(KDGpu::ShaderStageFlagBits::ComputeBit),
                    },
                    {
                            // Output storage buffer
                            .binding = 1,
                            .resourceType = KDGpu::ResourceBindingType::StorageBuffer,
                            .shaderStages = KDGpu::ShaderStageFlags(KDGpu::ShaderStageFlagBits::ComputeBit),
                    },
            },
    });
    //! [bindgrouplayout_compute]

    //! [bindgrouplayout_multiple_descriptors]
    KDGpu::BindGroupLayout layout2 = device.createBindGroupLayout(KDGpu::BindGroupLayoutOptions{
            .bindings = {
                    {
                            // Array of 16 textures
                            .binding = 0,
                            .count = 16,
                            .resourceType = KDGpu::ResourceBindingType::CombinedImageSampler,
                            .shaderStages = KDGpu::ShaderStageFlags(KDGpu::ShaderStageFlagBits::FragmentBit),
                    },
            },
    });
    //! [bindgrouplayout_multiple_descriptors]

    //! [bindgrouplayout_dynamic_ubo]
    KDGpu::BindGroupLayout layout3 = device.createBindGroupLayout(KDGpu::BindGroupLayoutOptions{
            .bindings = {
                    {
                            .binding = 0,
                            .resourceType = KDGpu::ResourceBindingType::DynamicUniformBuffer,
                            .shaderStages = KDGpu::ShaderStageFlags(KDGpu::ShaderStageFlagBits::VertexBit),
                    },
            },
    });

    // When binding, provide dynamic offset
    // renderPass.setBindGroup(0, bindGroup, { offsetInBuffer });
    //! [bindgrouplayout_dynamic_ubo]

    //! [bindgrouplayout_storage_image]
    KDGpu::BindGroupLayout layout4 = device.createBindGroupLayout(KDGpu::BindGroupLayoutOptions{
            .bindings = {
                    {
                            .binding = 0,
                            .resourceType = KDGpu::ResourceBindingType::StorageImage,
                            .shaderStages = KDGpu::ShaderStageFlags(KDGpu::ShaderStageFlagBits::ComputeBit),
                    },
            },
    });
    //! [bindgrouplayout_storage_image]

    //! [bindgrouplayout_bindless]
    KDGpu::BindGroupLayout bindlessLayout = device.createBindGroupLayout(KDGpu::BindGroupLayoutOptions{
            .bindings = {
                    {
                            .binding = 0,
                            .count = 1000, // Large descriptor array
                            .resourceType = KDGpu::ResourceBindingType::SampledImage,
                            .shaderStages = KDGpu::ShaderStageFlags(KDGpu::ShaderStageFlagBits::FragmentBit),
                    },
            },
    });
    //! [bindgrouplayout_bindless]

    //! [bindgrouplayout_shader_stages]
    KDGpu::BindGroupLayout layout5 = device.createBindGroupLayout(KDGpu::BindGroupLayoutOptions{
            .bindings = {
                    {
                            // Used in vertex shader only
                            .binding = 0,
                            .resourceType = KDGpu::ResourceBindingType::UniformBuffer,
                            .shaderStages = KDGpu::ShaderStageFlags(KDGpu::ShaderStageFlagBits::VertexBit),
                    },
                    {
                            // Used in both vertex and fragment
                            .binding = 1,
                            .resourceType = KDGpu::ResourceBindingType::UniformBuffer,
                            .shaderStages = KDGpu::ShaderStageFlags(KDGpu::ShaderStageFlagBits::VertexBit |
                                                                    KDGpu::ShaderStageFlagBits::FragmentBit),
                    },
                    {
                            // Fragment only
                            .binding = 2,
                            .resourceType = KDGpu::ResourceBindingType::CombinedImageSampler,
                            .shaderStages = KDGpu::ShaderStageFlags(KDGpu::ShaderStageFlagBits::FragmentBit),
                    },
            },
    });
    //! [bindgrouplayout_shader_stages]

    //! [bindgrouplayout_compatibility]
    // These layouts are compatible because bindings match
    KDGpu::BindGroupLayout layout1a = device.createBindGroupLayout(KDGpu::BindGroupLayoutOptions{
            .bindings = {
                    {
                            .binding = 0,
                            .resourceType = KDGpu::ResourceBindingType::UniformBuffer,
                            .shaderStages = KDGpu::ShaderStageFlags(KDGpu::ShaderStageFlagBits::VertexBit),
                    },
            },
    });

    KDGpu::BindGroupLayout layout2a = device.createBindGroupLayout(KDGpu::BindGroupLayoutOptions{
            .bindings = {
                    {
                            .binding = 0,
                            .resourceType = KDGpu::ResourceBindingType::UniformBuffer,
                            .shaderStages = KDGpu::ShaderStageFlags(KDGpu::ShaderStageFlagBits::VertexBit),
                    },
            },
    });

    // BindGroups created with layout1 can be used with pipelines created with layout2
    //! [bindgrouplayout_compatibility]

    //! [bindgrouplayout_shader_match]
    // Layout for set 0
    KDGpu::BindGroupLayout frameBindGroupLayout = device.createBindGroupLayout(KDGpu::BindGroupLayoutOptions{
            .bindings = {
                    {
                            .binding = 0,
                            .resourceType = KDGpu::ResourceBindingType::UniformBuffer,
                            .shaderStages = KDGpu::ShaderStageFlags(KDGpu::ShaderStageFlagBits::VertexBit),
                    },
            },
    });

    // Layout for set 1
    KDGpu::BindGroupLayout materialBindGroupLayout = device.createBindGroupLayout(KDGpu::BindGroupLayoutOptions{
            .bindings = {
                    {
                            .binding = 0,
                            .resourceType = KDGpu::ResourceBindingType::CombinedImageSampler,
                            .shaderStages = KDGpu::ShaderStageFlags(KDGpu::ShaderStageFlagBits::VertexBit),
                    },
                    {
                            .binding = 1,
                            .resourceType = KDGpu::ResourceBindingType::CombinedImageSampler,
                            .shaderStages = KDGpu::ShaderStageFlags(KDGpu::ShaderStageFlagBits::VertexBit),
                    },
                    {
                            .binding = 2,
                            .resourceType = KDGpu::ResourceBindingType::CombinedImageSampler,
                            .shaderStages = KDGpu::ShaderStageFlags(KDGpu::ShaderStageFlagBits::VertexBit),
                    },
            },
    });

    // Layout for set 2
    KDGpu::BindGroupLayout objectBindGroupLayout = device.createBindGroupLayout(KDGpu::BindGroupLayoutOptions{
            .bindings = {
                    {
                            .binding = 0,
                            .resourceType = KDGpu::ResourceBindingType::UniformBuffer,
                            .shaderStages = KDGpu::ShaderStageFlags(KDGpu::ShaderStageFlagBits::VertexBit),
                    },
            },
    });

    KDGpu::PipelineLayout pipelineLayout = device.createPipelineLayout(KDGpu::PipelineLayoutOptions{
            .bindGroupLayouts = {
                    frameBindGroupLayout,
                    materialBindGroupLayout,
                    objectBindGroupLayout,
            },
    });
    //! [bindgrouplayout_shader_match]
}

// =============================================================================
// BindGroupPool Examples (bind_group_pool.h)
// =============================================================================

void bindgrouppool_examples(KDGpu::Device &device)
{
    KDGpu::BindGroupLayout layout;

    //! [bindgrouppool_creation]
    KDGpu::BindGroupPool pool = device.createBindGroupPool(KDGpu::BindGroupPoolOptions{});
    //! [bindgrouppool_creation]

    //! [bindgrouppool_allocate]
    KDGpu::BindGroupPool pool2 = device.createBindGroupPool(KDGpu::BindGroupPoolOptions{});

    KDGpu::BindGroup bindGroup1 = device.createBindGroup(KDGpu::BindGroupOptions{
            .layout = layout,
    });
    KDGpu::BindGroup bindGroup2 = device.createBindGroup(KDGpu::BindGroupOptions{
            .layout = layout,
    });
    // ... allocate up to 100 bind groups
    //! [bindgrouppool_allocate]

    //! [bindgrouppool_reset]
    // Allocate and use bind groups for frame N
    for (int i = 0; i < 10; ++i) {
        KDGpu::BindGroup bg = device.createBindGroup(KDGpu::BindGroupOptions{
                .layout = layout,
        });
        // ... use bind group ...
    }

    // Frame complete, reset pool for next frame
    pool.reset();

    // Allocate bind groups for frame N+1 (reuses memory)
    for (int i = 0; i < 15; ++i) {
        KDGpu::BindGroup bg = device.createBindGroup(KDGpu::BindGroupOptions{
                .layout = layout,
        });
        // ... use bind group ...
    }
    //! [bindgrouppool_reset]

    //! [bindgrouppool_per_frame]
    struct PerFrameResources {
        KDGpu::BindGroupPool pool;
        // ... other per-frame resources ...
    };

    // std::array<PerFrameResources, 3> frameResources; // Triple buffering

    // void renderFrame(uint32_t frameIndex, KDGpu::Device &device, KDGpu::BindGroupLayout &layout)
    // {
    //     auto &frame = frameResources[frameIndex];
    //     frame.pool.clear(); // Clear previous frame's allocations
    //
    //     // Allocate bind groups for this frame
    //     KDGpu::BindGroup bindGroup = device.createBindGroup(KDGpu::BindGroupOptions{.layout = layout.handle()});
    //     // ... render ...
    // }
    //! [bindgrouppool_per_frame]

    //! [bindgrouppool_limits]
    KDGpu::BindGroupPool smallPool = device.createBindGroupPool(KDGpu::BindGroupPoolOptions{});

    for (int i = 0; i < 10; ++i) {
        KDGpu::BindGroup bg = device.createBindGroup(KDGpu::BindGroupOptions{
                .layout = layout,
        }); // OK
    }

    // This may fail or reallocate a larger pool (implementation-dependent)
    // KDGpu::BindGroup bg11 = device.createBindGroup(KDGpu::BindGroupOptions{.layout = layout.handle()});
    //! [bindgrouppool_limits]
}

// =============================================================================
// BindGroup Examples (bind_group.h)
// =============================================================================

void bindgroup_examples(KDGpu::Device &device)
{
    KDGpu::BindGroupLayout layout;
    KDGpu::Buffer uniformBuffer, storageBuffer;
    KDGpu::TextureView textureView;
    KDGpu::Sampler sampler;

    //! [bindgroup_simple]
    KDGpu::BindGroup bindGroup = device.createBindGroup(KDGpu::BindGroupOptions{
            .layout = layout,
            .resources = {
                    {
                            // Binding 0: uniform buffer
                            .binding = 0,
                            .resource = KDGpu::UniformBufferBinding{
                                    .buffer = uniformBuffer,
                            },
                    },
                    {
                            // Binding 1: texture + sampler
                            .binding = 1,
                            .resource = KDGpu::TextureViewSamplerBinding{
                                    .textureView = textureView,
                                    .sampler = sampler,
                            },
                    },
            },
    });
    //! [bindgroup_simple]

    //! [bindgroup_multiple_uniforms]
    KDGpu::BindGroup multipleUBOBindGroup = device.createBindGroup(KDGpu::BindGroupOptions{
            .layout = layout,
            .resources = {
                    {
                            .binding = 0,
                            .resource = KDGpu::UniformBufferBinding{
                                    .buffer = uniformBuffer,
                            },
                    },
                    {
                            .binding = 1,
                            .resource = KDGpu::UniformBufferBinding{
                                    .buffer = uniformBuffer,
                                    .offset = 256,
                                    .size = 64,
                            },
                    },
            },
    });
    //! [bindgroup_multiple_uniforms]

    //! [bindgroup_storage]
    KDGpu::BindGroup computeBindGroup = device.createBindGroup(KDGpu::BindGroupOptions{
            .layout = layout,
            .resources = {
                    {
                            .binding = 0,
                            .resource = KDGpu::StorageBufferBinding{
                                    .buffer = storageBuffer,
                            },
                    },
            },
    });
    //! [bindgroup_storage]

    //! [bindgroup_texture_view_sampler_separate]
    KDGpu::BindGroup textureAndSamplerBindGroup = device.createBindGroup(KDGpu::BindGroupOptions{
            .layout = layout,
            .resources = {
                    {
                            .binding = 0,
                            .resource = KDGpu::TextureViewBinding{
                                    .textureView = textureView,
                            },
                    },
                    {
                            .binding = 1,
                            .resource = KDGpu::SamplerBinding{
                                    .sampler = sampler,
                            },
                    },
            },
    });
    //! [bindgroup_texture_view_sampler_separate]

    KDGpu::TextureView colorView;
    //! [bindgroup_usage]
    KDGpu::CommandRecorder cmdRecorder = device.createCommandRecorder();

    // In render pass
    KDGpu::RenderPassCommandRecorder renderPass = cmdRecorder.beginRenderPass(KDGpu::RenderPassCommandRecorderOptions{
            /* ... */
    });
    renderPass.setBindGroup(0, bindGroup);
    renderPass.end();

    // In compute pass
    KDGpu::ComputePassCommandRecorder computePass = cmdRecorder.beginComputePass();
    computePass.setBindGroup(0, bindGroup);
    computePass.end();
    //! [bindgroup_usage]

    //! [bindgroup_resource_bound_after_creation]
    KDGpu::BindGroup emptyBindGroup = device.createBindGroup(KDGpu::BindGroupOptions{
            .layout = layout,
    });

    // Later when resource is available (and before bindgroup is used in a draw or compute call)
    emptyBindGroup.update(KDGpu::BindGroupEntry{
            .binding = 0,
            .resource = KDGpu::UniformBufferBinding{
                    .buffer = uniformBuffer,
            },
    });
    //! [bindgroup_resource_bound_after_creation]

    //! [bindgroup_dynamic_offset]
    const uint32_t dynamicUBOStride = 16 * sizeof(float);
    // Create bind group with dynamic uniform buffer
    KDGpu::BindGroup dynamicBindGroup = device.createBindGroup(KDGpu::BindGroupOptions{
            .layout = layout,
            .resources = {
                    {
                            .binding = 0,
                            .resource = KDGpu::DynamicUniformBufferBinding{
                                    .buffer = uniformBuffer,
                                    .size = dynamicUBOStride,
                            },
                    },
            },
    });

    // Use with different offsets for different draws
    const std::array<uint32_t, 1> offset0 = { 0 };
    renderPass.setBindGroup(0, dynamicBindGroup, {}, offset0); // Offset 0
    /* Issue some draw calls */

    const std::array<uint32_t, 1> offset1 = { dynamicUBOStride };
    renderPass.setBindGroup(0, dynamicBindGroup, {}, offset1); // Offset 1
    /* Issue some more draw calls */

    //! [bindgroup_dynamic_offset]

    //! [bindgroup_update]
    // Initial bind group
    KDGpu::BindGroup bgUpdatable = device.createBindGroup(KDGpu::BindGroupOptions{
            .layout = layout,
            .resources = {
                    {
                            .binding = 0,
                            .resource = KDGpu::UniformBufferBinding{
                                    .buffer = uniformBuffer,
                            },
                    },
            },
    });

    // New Uniform Buffer
    KDGpu::Buffer newUniformBuffer = device.createBuffer(KDGpu::BufferOptions{
            /* ... */
    });

    // Update BindGroup to reference new buffer
    bgUpdatable.update(KDGpu::BindGroupEntry{
            .binding = 0,
            .resource = KDGpu::UniformBufferBinding{
                    .buffer = newUniformBuffer,
            },
    });
    //! [bindgroup_update]

    KDGpu::BindGroup bindGroup0,
            bindGroup1, bindGroup2;

    KDGpu::BindGroupLayout bindGroupLayout0, bindGroupLayout1, bindGroupLayout2;

    //! [bindgroup_pipeline_layout]
    KDGpu::PipelineLayout pipelineLayoutMultiBindGroups = device.createPipelineLayout(KDGpu::PipelineLayoutOptions{
            .bindGroupLayouts = {
                    bindGroupLayout0,
                    bindGroupLayout1,
                    bindGroupLayout2,
            },
    });

    // At render time, bind to set 0, 1, 2
    renderPass.setBindGroup(0, bindGroup0);
    renderPass.setBindGroup(1, bindGroup1);
    renderPass.setBindGroup(2, bindGroup2);
    //! [bindgroup_pipeline_layout]
}

// =============================================================================
// CommandRecorder Examples (command_recorder.h)
// =============================================================================

void commandrecorder_examples(KDGpu::Device &device)
{
    //! [commandrecorder_creation]
    KDGpu::CommandRecorder recorder = device.createCommandRecorder();
    //! [commandrecorder_creation]

    KDGpu::TextureView colorView, depthView;

    //! [commandrecorder_render_pass]
    auto renderPass = recorder.beginRenderPass(KDGpu::RenderPassCommandRecorderOptions{
            .colorAttachments = {
                    {
                            .view = colorView,
                            .loadOperation = KDGpu::AttachmentLoadOperation::Clear,
                            .clearValue = { 0.2f, 0.3f, 0.3f, 1.0f },
                    },
            },
            .depthStencilAttachment = {
                    .view = depthView,
            },
    });

    // ... record drawing commands ...
    renderPass.end();
    //! [commandrecorder_render_pass]

    //! [commandrecorder_compute_pass]
    auto computePass = recorder.beginComputePass();
    // ... record compute commands ...
    computePass.end();
    //! [commandrecorder_compute_pass]

    KDGpu::Buffer srcBuffer, dstBuffer;
    KDGpu::Texture srcTexture, dstTexture;

    //! [commandrecorder_copy_buffer]
    recorder.copyBuffer(KDGpu::BufferCopy{
            .src = srcBuffer,
            .dst = dstBuffer,
            .byteSize = 1024,
    });
    //! [commandrecorder_copy_buffer]

    //! [commandrecorder_copy_texture]
    recorder.copyTextureToTexture(KDGpu::TextureToTextureCopy{
            .srcTexture = srcTexture.handle(),
            .dstTexture = dstTexture.handle(),
            .regions = {
                    {
                            .extent = { 512, 512, 1 },
                    },
            },
    });
    //! [commandrecorder_copy_texture]

    KDGpu::Queue queue;

    //! [commandrecorder_submit]
    auto submitRenderPass = recorder.beginRenderPass(KDGpu::RenderPassCommandRecorderOptions{
            .colorAttachments = {
                    {
                            .view = colorView,
                    },
            },
    });
    // ... record commands ...
    submitRenderPass.end();

    KDGpu::CommandBuffer commandBuffer = recorder.finish();
    queue.submit(KDGpu::SubmitOptions{
            .commandBuffers = { commandBuffer },
    });
    //! [commandrecorder_submit]

    //! [commandrecorder_reuse]
    KDGpu::CommandRecorder recorderReuse = device.createCommandRecorder();

    // Frame 1
    auto pass1 = recorderReuse.beginRenderPass(KDGpu::RenderPassCommandRecorderOptions{
            .colorAttachments = {
                    {
                            .view = colorView,
                    },
            },
    });
    pass1.end();
    KDGpu::CommandBuffer cmdBuffer1 = recorderReuse.finish();
    queue.submit(KDGpu::SubmitOptions{
            .commandBuffers = { cmdBuffer1 },
    });

    // Frame 2 - reuse recorder
    auto pass2 = recorderReuse.beginRenderPass(KDGpu::RenderPassCommandRecorderOptions{
            .colorAttachments = { {
                    .view = colorView,
            } },
    });
    pass2.end();
    KDGpu::CommandBuffer cmdBuffer2 = recorderReuse.finish();
    queue.submit(KDGpu::SubmitOptions{
            .commandBuffers = { cmdBuffer2 },
    });
    //! [commandrecorder_reuse]
}

// =============================================================================
// ComputePipeline Examples (compute_pipeline.h)
// =============================================================================

void computepipeline_examples(KDGpu::Device &device)
{
    KDGpu::ShaderModule computeShader;
    KDGpu::PipelineLayout pipelineLayout;

    //! [computepipeline_creation]
    KDGpu::ComputePipeline pipeline = device.createComputePipeline(KDGpu::ComputePipelineOptions{
            .layout = pipelineLayout,
            .shaderStage = {
                    .shaderModule = computeShader,
            },
    });
    //! [computepipeline_creation]

    KDGpu::CommandRecorder recorder = device.createCommandRecorder();
    KDGpu::BindGroup bindGroup;

    //! [computepipeline_dispatch]
    auto computePass = recorder.beginComputePass();
    computePass.setPipeline(pipeline);
    computePass.setBindGroup(0, bindGroup);
    computePass.dispatchCompute(KDGpu::ComputeCommand{
            .workGroupX = 256,
            .workGroupY = 1,
            .workGroupZ = 1,
    });
    computePass.end();
    //! [computepipeline_dispatch]

    //! [computepipeline_particle_system]
    // Compute shader updates particle positions
    KDGpu::ComputePipeline particlePipeline = device.createComputePipeline(KDGpu::ComputePipelineOptions{
            .layout = pipelineLayout,
            .shaderStage = {
                    .shaderModule = computeShader,
            },
    });

    uint32_t particleCount = 10000;
    uint32_t workGroupSize = 256;
    uint32_t workGroupCount = (particleCount + workGroupSize - 1) / workGroupSize;

    auto particleComputePass = recorder.beginComputePass();
    particleComputePass.setPipeline(particlePipeline);
    particleComputePass.setBindGroup(0, bindGroup);
    particleComputePass.dispatchCompute(KDGpu::ComputeCommand{
            .workGroupX = workGroupCount,
    });
    particleComputePass.end();
    //! [computepipeline_particle_system]

    //! [computepipeline_specialization]
    KDGpu::ComputePipeline specializationPipeline = device.createComputePipeline(KDGpu::ComputePipelineOptions{
            .layout = pipelineLayout.handle(),
            .shaderStage = {
                    .shaderModule = computeShader.handle(),
                    .specializationConstants = {
                            { .constantId = 0, .value = 256u }, // WORK_GROUP_SIZE
                            { .constantId = 1, .value = 1.0f }, // TIME_STEP
                    },
            },
    });
    //! [computepipeline_specialization]

    //! [computepipeline_image_processing]
    // Image processing compute shader
    KDGpu::Texture inputTexture, outputTexture;
    KDGpu::TextureView inputView = inputTexture.createView();
    KDGpu::TextureView outputView = outputTexture.createView();

    KDGpu::BindGroupLayout imageLayout = device.createBindGroupLayout(KDGpu::BindGroupLayoutOptions{
            .bindings = {
                    {
                            .binding = 0,
                            .resourceType = KDGpu::ResourceBindingType::SampledImage,
                    },
                    {
                            .binding = 1,
                            .resourceType = KDGpu::ResourceBindingType::StorageImage,
                    },
            },
    });

    KDGpu::BindGroup imageBindGroup = device.createBindGroup(KDGpu::BindGroupOptions{
            .layout = imageLayout,
            .resources = {
                    {
                            .binding = 0,
                            .resource = KDGpu::TextureViewBinding{
                                    .textureView = inputView,
                            },
                    },
                    {
                            .binding = 1,
                            .resource = KDGpu::ImageBinding{
                                    .textureView = outputView,
                            },
                    },
            },
    });

    uint32_t imageWidth = 1920, imageHeight = 1080;
    uint32_t localSizeX = 16, localSizeY = 16;

    auto imageComputePass = recorder.beginComputePass();
    imageComputePass.setPipeline(pipeline);
    imageComputePass.setBindGroup(0, imageBindGroup);
    imageComputePass.dispatchCompute(KDGpu::ComputeCommand{
            .workGroupX = (imageWidth + localSizeX - 1) / localSizeX,
            .workGroupY = (imageHeight + localSizeY - 1) / localSizeY,
            .workGroupZ = 1,
    });
    imageComputePass.end();
    //! [computepipeline_image_processing]
}

// =============================================================================
// Device Examples (device.h)
// =============================================================================

void device_examples(KDGpu::Adapter &adapter)
{
    //! [device_creation]
    KDGpu::Device device = adapter.createDevice(KDGpu::DeviceOptions{
            .requestedFeatures = {
                    .geometryShader = true,
                    .tessellationShader = true,
            },
    });
    //! [device_creation]

    //! [device_queue]
    KDGpu::Queue queue = device.queues()[0]; // Get first queue
    //! [device_queue]

    //! [device_create_buffer]
    KDGpu::Buffer buffer = device.createBuffer(KDGpu::BufferOptions{
            .size = 1024,
            .usage = KDGpu::BufferUsageFlagBits::VertexBufferBit,
            .memoryUsage = KDGpu::MemoryUsage::CpuToGpu,
    });
    //! [device_create_buffer]

    //! [device_create_texture]
    KDGpu::Texture texture = device.createTexture(KDGpu::TextureOptions{
            .type = KDGpu::TextureType::TextureType2D,
            .format = KDGpu::Format::R8G8B8A8_SRGB,
            .extent = { 1024, 1024, 1 },
            .mipLevels = 1,
            .usage = KDGpu::TextureUsageFlagBits::SampledBit,
    });
    //! [device_create_texture]

    //! [device_wait_idle]
    // Finish all pending GPU work
    device.waitUntilIdle();
    //! [device_wait_idle]

    //! [device_multi_queue]
    KDGpu::Device deviceMultiQueue = adapter.createDevice(KDGpu::DeviceOptions{
            .queues = {
                    { .queueTypeIndex = 0, .count = 1 }, // Graphics queue
                    { .queueTypeIndex = 1, .count = 1 }, // Compute queue
            },
    });

    KDGpu::Queue graphicsQueue = device.queues()[0];
    KDGpu::Queue computeQueue = device.queues()[1];
    //! [device_multi_queue]
}

// =============================================================================
// Fence Examples (fence.h)
// =============================================================================

void fence_examples(KDGpu::Device &device, KDGpu::Queue &queue)
{
    //! [fence_creation]
    KDGpu::Fence fence = device.createFence();
    //! [fence_creation]

    KDGpu::CommandBuffer commandBuffer;

    //! [fence_submit_wait]
    queue.submit(KDGpu::SubmitOptions{
            .commandBuffers = { commandBuffer },
            .signalFence = fence,
    });

    fence.wait(); // Block until GPU work completes
    //! [fence_submit_wait]

    //! [fence_status]
    queue.submit(KDGpu::SubmitOptions{
            .commandBuffers = { commandBuffer },
            .signalFence = fence,
    });

    // Do other work while GPU executes...

    if (fence.status() == KDGpu::FenceStatus::Signalled) {
        std::cout << "GPU work complete" << std::endl;
    } else {
        std::cout << "GPU still working" << std::endl;
    }
    //! [fence_status]

    //! [fence_cpu_gpu_sync]
    // Example showing pattern for synchronizing CPU and GPU per frame
    KDGpu::Fence frameFence = device.createFence(KDGpu::FenceOptions{
            .createSignalled = true, // allowing us to wait initially
    });

    auto renderFrame = [&]() {
        // 1. Wait for previous frame to complete
        frameFence.wait();

        // 2. Update resources
        // ... upload new data ...

        // 3. Record and submit commands
        KDGpu::CommandRecorder recorder = device.createCommandRecorder();
        // ... record rendering ...
        KDGpu::CommandBuffer cmdBuffer = recorder.finish();

        frameFence.reset(); // Unsignal fence
        queue.submit(KDGpu::SubmitOptions{
                .commandBuffers = { cmdBuffer },
                .signalFence = frameFence,
        });
    };

    while (true) {
        renderFrame();
    }
    //! [fence_cpu_gpu_sync]

    //! [fence_reset]
    fence.wait(); // Wait for signal
    fence.reset(); // Return to unsignaled state
    queue.submit(KDGpu::SubmitOptions{
            .commandBuffers = { commandBuffer },
            .signalFence = fence, // Reuse fence
    });
    //! [fence_reset]
}

// =============================================================================
// GpuSemaphore Examples (gpu_semaphore.h)
// =============================================================================

void gpusemaphore_examples(KDGpu::Device &device, KDGpu::Queue &graphicsQueue, KDGpu::Queue &computeQueue)
{
    //! [gpusemaphore_creation]
    KDGpu::GpuSemaphore semaphore = device.createGpuSemaphore();
    //! [gpusemaphore_creation]

    KDGpu::CommandBuffer computeCommands, graphicsCommands;

    //! [gpusemaphore_queue_sync]
    // Submit compute work, signal semaphore when done
    computeQueue.submit(KDGpu::SubmitOptions{
            .commandBuffers = { computeCommands },
            .signalSemaphores = { semaphore },
    });

    // Wait for compute before starting graphics
    graphicsQueue.submit(KDGpu::SubmitOptions{
            .commandBuffers = { graphicsCommands },
            .waitSemaphores = { semaphore },
    });
    //! [gpusemaphore_queue_sync]

    KDGpu::Swapchain swapchain;
    KDGpu::GpuSemaphore imageAvailableSemaphore = device.createGpuSemaphore();
    KDGpu::GpuSemaphore renderCompleteSemaphore = device.createGpuSemaphore();

    //! [gpusemaphore_swapchain]
    uint32_t imageIndex;
    // getNextImageIndex will signal imageAvailableSemaphore
    KDGpu::AcquireImageResult result = swapchain.getNextImageIndex(imageIndex, imageAvailableSemaphore);

    graphicsQueue.submit(KDGpu::SubmitOptions{
            .commandBuffers = { graphicsCommands },
            .waitSemaphores = { imageAvailableSemaphore }, // awaits imageAvailableSemaphore
            .signalSemaphores = { renderCompleteSemaphore }, // signals renderCompleteSemaphore
    });

    graphicsQueue.present(KDGpu::PresentOptions{
            .waitSemaphores = { renderCompleteSemaphore }, // awaits renderCompleteSemaphore
            .swapchainInfos = {
                    {
                            .swapchain = swapchain,
                            .imageIndex = imageIndex,
                    },
            },
    });
    //! [gpusemaphore_swapchain]

    KDGpu::CommandBuffer geometryCommands, shadingCommands, postProcessCommands;

    //! [gpusemaphore_multi_queue]
    KDGpu::GpuSemaphore geometryDone = device.createGpuSemaphore();
    KDGpu::GpuSemaphore shadingDone = device.createGpuSemaphore();

    // Geometry processing on compute queue
    computeQueue.submit(KDGpu::SubmitOptions{
            .commandBuffers = { geometryCommands },
            .signalSemaphores = { geometryDone },
    });

    // Shading on graphics queue, waits for geometry
    graphicsQueue.submit(KDGpu::SubmitOptions{
            .commandBuffers = { shadingCommands },
            .waitSemaphores = { geometryDone },
            .signalSemaphores = { shadingDone },
    });

    // Post-processing, waits for shading
    graphicsQueue.submit(KDGpu::SubmitOptions{
            .commandBuffers = { postProcessCommands },
            .waitSemaphores = { shadingDone },
    });
    //! [gpusemaphore_multi_queue]
}

// =============================================================================
// GraphicsPipeline Examples (graphics_pipeline.h)
// =============================================================================

void graphicspipeline_examples(KDGpu::Device &device)
{
    KDGpu::ShaderModule vertexShader, fragmentShader;
    KDGpu::PipelineLayout pipelineLayout;
    KDGpu::Format swapchainFormat = KDGpu::Format::B8G8R8A8_SRGB;

    //! [graphicspipeline_simple]
    KDGpu::GraphicsPipeline pipeline = device.createGraphicsPipeline(KDGpu::GraphicsPipelineOptions{
            .shaderStages = {
                    {
                            .shaderModule = vertexShader,
                            .stage = KDGpu::ShaderStageFlagBits::VertexBit,
                    },
                    {
                            .shaderModule = fragmentShader,
                            .stage = KDGpu::ShaderStageFlagBits::FragmentBit,
                    },
            },
            .layout = pipelineLayout.handle(),
            .vertex = {
                    .buffers = {
                            {
                                    .binding = 0,
                                    .stride = sizeof(Vertex),
                            },
                    },
                    .attributes = {
                            {
                                    .location = 0,
                                    .binding = 0,
                                    .format = KDGpu::Format::R32G32B32_SFLOAT,
                                    .offset = offsetof(Vertex, position),
                            },
                            {
                                    .location = 1,
                                    .binding = 0,
                                    .format = KDGpu::Format::R32G32B32_SFLOAT,
                                    .offset = offsetof(Vertex, color),
                            },
                    },
            },
            .renderTargets = {
                    {
                            .format = swapchainFormat,
                    },
            },
    });
    //! [graphicspipeline_simple]

    //! [graphicspipeline_depth]
    KDGpu::GraphicsPipeline depthPipeline = device.createGraphicsPipeline(KDGpu::GraphicsPipelineOptions{
            .shaderStages = {
                    {
                            .shaderModule = vertexShader,
                            .stage = KDGpu::ShaderStageFlagBits::VertexBit,
                    },
                    {
                            .shaderModule = fragmentShader,
                            .stage = KDGpu::ShaderStageFlagBits::FragmentBit,
                    },
            },
            .layout = pipelineLayout,
            .vertex = { /* ... */ },
            .renderTargets = { {
                    .format = swapchainFormat,
            } },
            .depthStencil = {
                    .format = KDGpu::Format::D32_SFLOAT,
                    .depthWritesEnabled = true,
                    .depthCompareOperation = KDGpu::CompareOperation::Less,
            },
    });
    //! [graphicspipeline_depth]

    //! [graphicspipeline_blending]
    KDGpu::GraphicsPipeline blendPipeline = device.createGraphicsPipeline(KDGpu::GraphicsPipelineOptions{
            .shaderStages = { /* ... */ },
            .layout = pipelineLayout,
            .vertex = { /* ... */ },
            .renderTargets = {
                    KDGpu::RenderTargetOptions{
                            .format = swapchainFormat,
                            .blending = KDGpu::BlendOptions{
                                    .blendingEnabled = true,
                                    .color = KDGpu::BlendComponent{
                                            .operation = KDGpu::BlendOperation::Add,
                                            .srcFactor = KDGpu::BlendFactor::SrcAlpha,
                                            .dstFactor = KDGpu::BlendFactor::OneMinusSrcAlpha,
                                    },
                                    .alpha = KDGpu::BlendComponent{
                                            .operation = KDGpu::BlendOperation::Add,
                                            .srcFactor = KDGpu::BlendFactor::One,
                                            .dstFactor = KDGpu::BlendFactor::Zero,
                                    },
                            },
                    },
            },
    });
    //! [graphicspipeline_blending]

    //! [graphicspipeline_culling]
    KDGpu::GraphicsPipeline cullPipeline = device.createGraphicsPipeline(KDGpu::GraphicsPipelineOptions{
            .shaderStages = { /* ... */ },
            .layout = pipelineLayout,
            .vertex = { /* ... */ },
            .renderTargets = {
                    {
                            .format = swapchainFormat,
                    },
            },
            .primitive = {
                    .topology = KDGpu::PrimitiveTopology::TriangleList,
                    .cullMode = KDGpu::CullModeFlagBits::BackBit,
                    .frontFace = KDGpu::FrontFace::CounterClockwise,
            },
    });
    //! [graphicspipeline_culling]

    //! [graphicspipeline_multisampling]
    KDGpu::GraphicsPipeline msaaPipeline = device.createGraphicsPipeline(KDGpu::GraphicsPipelineOptions{
            .shaderStages = { /* ... */ },
            .layout = pipelineLayout,
            .vertex = { /* ... */ },
            .renderTargets = { {
                    .format = swapchainFormat,
            } },
            .multisample = {
                    .samples = KDGpu::SampleCountFlagBits::Samples4Bit,
            },
    });
    //! [graphicspipeline_multisampling]
}

// =============================================================================
// Instance Examples (instance.h)
// =============================================================================

void instance_examples()
{
    //! [instance_creation]
    // Note: Instance creation requires a GraphicsApi (VulkanGraphicsApi)
    std::unique_ptr<KDGpu::GraphicsApi> api = std::make_unique<KDGpu::VulkanGraphicsApi>();

    KDGpu::Instance instance = api->createInstance(KDGpu::InstanceOptions{
            .applicationName = "MyApplication",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0),
    });
    //! [instance_creation]

    //! [instance_adapters]
    std::vector<KDGpu::Adapter *> adapters = instance.adapters();
    // Iterate over adapters, use properties to gather information about each adapter
    for (auto *adapter : adapters) {
        std::cout << "Found GPU: " << adapter->properties().deviceName
                  << KDGpu::adapterDeviceTypeToString(adapter->properties().deviceType)
                  << std::endl;
    }
    //! [instance_adapters]

    //! [instance_extensions]
    // Extensions are configured through InstanceOptions
    KDGpu::Instance instanceWithExtensions = api->createInstance(KDGpu::InstanceOptions{
            .layers = { "VK_LAYER_KHRONOS_validation" },
            .extensions = { VK_KHR_SURFACE_EXTENSION_NAME },
    });

    for (const KDGpu::Extension &ext : instanceWithExtensions.extensions()) {
        if (ext.name == VK_KHR_SURFACE_EXTENSION_NAME) {
            /* ... */
            break;
        }
    }
    //! [instance_extensions]

    //! [instance_surface]
    // Platform-specific (example for XCB on Linux)
    // xcb_connection_t *connection = /* ... */;
    // xcb_window_t window = /* ... */;
    KDGpu::Surface surface = instance.createSurface(KDGpu::SurfaceOptions{
            /* Set underlying window handle based on platform */
    });
    //! [instance_surface]

    //! [instance_api_version]
    // API version is configured through InstanceOptions
    KDGpu::Instance instanceWithApiVersion = api->createInstance(KDGpu::InstanceOptions{
            .apiVersion = KDGPU_MAKE_API_VERSION(0, 1, 2, 0),
    });
    //! [instance_api_version]
}

// =============================================================================
// PipelineLayout Examples (pipeline_layout.h)
// =============================================================================

void pipelinelayout_examples(KDGpu::Device &device, KDGpu::RenderPassCommandRecorder &renderPass)
{
    //! [pipelinelayout_simple]
    KDGpu::BindGroupLayout bindGroupLayout = device.createBindGroupLayout(KDGpu::BindGroupLayoutOptions{
            /* ... */
    });
    KDGpu::PipelineLayout layoutSimple = device.createPipelineLayout(KDGpu::PipelineLayoutOptions{
            .bindGroupLayouts = { bindGroupLayout },
    });
    //! [pipelinelayout_simple]

    //! [pipelinelayout_multiple_sets]
    KDGpu::BindGroupLayout set0Layout = device.createBindGroupLayout(KDGpu::BindGroupLayoutOptions{
            /* ... */
    });
    KDGpu::BindGroupLayout set1Layout = device.createBindGroupLayout(KDGpu::BindGroupLayoutOptions{
            /* ... */
    });
    KDGpu::BindGroupLayout set2Layout = device.createBindGroupLayout(KDGpu::BindGroupLayoutOptions{
            /* ... */
    });
    KDGpu::PipelineLayout layoutMulti = device.createPipelineLayout(KDGpu::PipelineLayoutOptions{
            .bindGroupLayouts = {
                    set0Layout, // Set 0: per-frame uniforms
                    set1Layout, // Set 1: per-material textures
                    set2Layout, // Set 2: per-object data
            },
    });
    //! [pipelinelayout_multiple_sets]

    //! [pipelinelayout_push_constants]
    const KDGpu::PushConstantRange myPushConstantRange = {
        .offset = 0,
        .size = sizeof(glm::mat4),
        .shaderStages = KDGpu::ShaderStageFlags(KDGpu::ShaderStageFlagBits::VertexBit),
    };

    KDGpu::PipelineLayout layoutPush = device.createPipelineLayout(KDGpu::PipelineLayoutOptions{
            .bindGroupLayouts = { bindGroupLayout.handle() },
            .pushConstantRanges = {
                    myPushConstantRange },
    });

    // Usage in render pass:
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    renderPass.pushConstant(myPushConstantRange, &modelMatrix);
    //! [pipelinelayout_push_constants]

    //! [pipelinelayout_compatibility]
    // Layouts are compatible if:
    // 1. They have the same number of descriptor sets
    // 2. Each set has compatible bind group layouts

    KDGpu::PipelineLayout layoutCompat1 = device.createPipelineLayout(KDGpu::PipelineLayoutOptions{
            .bindGroupLayouts = { set0Layout, set1Layout },
    });

    KDGpu::PipelineLayout layoutCompat2 = device.createPipelineLayout(KDGpu::PipelineLayoutOptions{
            .bindGroupLayouts = { set0Layout, set1Layout },
    });

    // These layouts are compatible - bind groups from layout1 can be used with pipeline created from layout2
    //! [pipelinelayout_compatibility]

    //! [pipelinelayout_partial_update]
    // Partial bind group updates example
    // Note: Requires actual renderPass from beginRenderPass()
    //! [pipelinelayout_partial_update]
}

// =============================================================================
// Queue Examples (queue.h)
// =============================================================================

void queue_examples(KDGpu::Device &device)
{
    KDGpu::Queue queue = device.queues()[0];
    KDGpu::CommandBuffer commandBuffer;

    //! [queue_submit]
    queue.submit(KDGpu::SubmitOptions{
            .commandBuffers = { commandBuffer },
    });
    //! [queue_submit]

    KDGpu::Fence fence;

    //! [queue_submit_fence]
    queue.submit(KDGpu::SubmitOptions{
            .commandBuffers = { commandBuffer },
            .signalFence = fence,
    });

    // Do other work...
    fence.wait(); // Block until GPU completes
    //! [queue_submit_fence]

    KDGpu::GpuSemaphore semaphoreA;
    KDGpu::GpuSemaphore semaphoreB;

    //! [queue_submit_semaphore]
    queue.submit(KDGpu::SubmitOptions{
            .commandBuffers = { commandBuffer },
            .waitSemaphores = { semaphoreA },
            .signalSemaphores = { semaphoreB },
    });
    //! [queue_submit_semaphore]

    KDGpu::Swapchain swapchain;
    uint32_t imageIndex = 0;

    //! [queue_present]
    queue.present(KDGpu::PresentOptions{
            .waitSemaphores = { semaphoreA },
            .swapchainInfos = {
                    {
                            .swapchain = swapchain,
                            .imageIndex = imageIndex,
                    },
            },
    });
    //! [queue_present]

    //! [queue_wait_idle]
    queue.waitUntilIdle(); // Block until queue is empty
    //! [queue_wait_idle]
}

// =============================================================================
// ShaderModule Examples (shader_module.h)
// =============================================================================

void shadermodule_examples(KDGpu::Device &device)
{
    //! [shadermodule_from_spirv_file]
    std::vector<uint32_t> spirvCode = loadSpirv("shader.vert.spv");
    KDGpu::ShaderModule shader = device.createShaderModule(spirvCode);
    //! [shadermodule_from_spirv_file]

    //! [shadermodule_from_spirv_memory]
    std::vector<uint32_t> vertexSpirv = { /* SPIR-V bytecode */ };
    KDGpu::ShaderModule vertexShader = device.createShaderModule(vertexSpirv);
    //! [shadermodule_from_spirv_memory]

    //! [shadermodule_pipeline_usage]
    KDGpu::ShaderModule vertShader = device.createShaderModule(loadSpirv("shader.vert.spv"));
    KDGpu::ShaderModule fragShader = device.createShaderModule(loadSpirv("shader.frag.spv"));

    KDGpu::PipelineLayout pipelineLayout;
    KDGpu::GraphicsPipeline pipelineShaders = device.createGraphicsPipeline(KDGpu::GraphicsPipelineOptions{
            .shaderStages = {
                    {
                            .shaderModule = vertShader,
                            .stage = KDGpu::ShaderStageFlagBits::VertexBit,
                    },
                    {
                            .shaderModule = fragShader,
                            .stage = KDGpu::ShaderStageFlagBits::FragmentBit,
                    },
            },
            .layout = pipelineLayout,
            // ... other pipeline options ...
    });
    //! [shadermodule_pipeline_usage]

    //! [shadermodule_compute]
    KDGpu::ShaderModule computeShader = device.createShaderModule(loadSpirv("compute.comp.spv"));

    KDGpu::ComputePipeline computePipeline = device.createComputePipeline(KDGpu::ComputePipelineOptions{
            .layout = pipelineLayout,
            .shaderStage = {
                    .shaderModule = computeShader,
            },
    });
    //! [shadermodule_compute]

    //! [shadermodule_specialization]
    KDGpu::ShaderModule shaderSpec = device.createShaderModule(loadSpirv("shader.frag.spv"));

    KDGpu::GraphicsPipeline pipelineSpec = device.createGraphicsPipeline(KDGpu::GraphicsPipelineOptions{
            .shaderStages = {
                    {
                            .shaderModule = shaderSpec.handle(),
                            .stage = KDGpu::ShaderStageFlagBits::FragmentBit,
                            .specializationConstants = {
                                    { .constantId = 0, .value = 4u }, // NUM_LIGHTS
                                    { .constantId = 1, .value = true }, // ENABLE_SHADOWS
                                    { .constantId = 2, .value = 2.2f }, // GAMMA
                            },
                    },
            },
            .layout = pipelineLayout,
            // ... other options ...
    });
    //! [shadermodule_specialization]
}

// =============================================================================
// Texture Examples (texture.h)
// =============================================================================

void texture_examples(KDGpu::Device &device)
{
    //! [texture_2d_creation]
    KDGpu::Texture texture = device.createTexture(KDGpu::TextureOptions{
            .type = KDGpu::TextureType::TextureType2D,
            .format = KDGpu::Format::R8G8B8A8_SRGB,
            .extent = { 1024, 1024, 1 },
            .mipLevels = 1,
            .usage = KDGpu::TextureUsageFlagBits::SampledBit | KDGpu::TextureUsageFlagBits::TransferDstBit,
            .memoryUsage = KDGpu::MemoryUsage::GpuOnly,
    });
    //! [texture_2d_creation]

    //! [texture_mipmaps]
    uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(1024, 1024)))) + 1;

    KDGpu::Texture mipmappedTexture = device.createTexture(KDGpu::TextureOptions{
            .type = KDGpu::TextureType::TextureType2D,
            .format = KDGpu::Format::R8G8B8A8_SRGB,
            .extent = { 1024, 1024, 1 },
            .mipLevels = mipLevels,
            .usage = KDGpu::TextureUsageFlagBits::SampledBit | KDGpu::TextureUsageFlagBits::TransferDstBit,
    });
    //! [texture_mipmaps]

    //! [texture_cubemap]
    KDGpu::Texture cubemap = device.createTexture(KDGpu::TextureOptions{
            .type = KDGpu::TextureType::TextureTypeCube,
            .format = KDGpu::Format::R16G16B16A16_SFLOAT,
            .extent = { 512, 512, 1 },
            .mipLevels = 1,
            .arrayLayers = 6, // 6 cube faces
            .usage = KDGpu::TextureUsageFlagBits::SampledBit | KDGpu::TextureUsageFlagBits::TransferDstBit,
    });
    //! [texture_cubemap]

    KDGpu::CommandRecorder recorder = device.createCommandRecorder();
    KDGpu::Buffer stagingBuffer;
    std::vector<uint8_t> imageData = loadImageFile("texture.png");

    //! [texture_upload]
    // 1. Create staging buffer with image data
    KDGpu::Buffer textureStagingBuffer = device.createBuffer(KDGpu::BufferOptions{
                                                                     .size = imageData.size(),
                                                                     .usage = KDGpu::BufferUsageFlagBits::TransferSrcBit,
                                                                     .memoryUsage = KDGpu::MemoryUsage::CpuToGpu,
                                                             },
                                                             imageData.data());

    // 2. Copy to texture
    KDGpu::CommandRecorder uploadRecorder = device.createCommandRecorder();
    uploadRecorder.copyBufferToTexture(KDGpu::BufferToTextureCopy{
            .srcBuffer = textureStagingBuffer,
            .dstTexture = texture,
            .dstTextureLayout = KDGpu::TextureLayout::TransferDstOptimal,
            .regions = {
                    {
                            .textureSubResource = {
                                    .aspectMask = KDGpu::TextureAspectFlagBits::ColorBit,
                                    .mipLevel = 0,
                            },
                            .textureExtent = { 1024, 1024, 1 },
                    },
            },
    });

    // 3. Transition to shader-readable layout
    uploadRecorder.textureMemoryBarrier(KDGpu::TextureMemoryBarrierOptions{
            .srcStages = KDGpu::PipelineStageFlagBit::TransferBit,
            .srcMask = KDGpu::AccessFlagBit::TransferWriteBit,
            .dstStages = KDGpu::PipelineStageFlagBit::FragmentShaderBit,
            .dstMask = KDGpu::AccessFlagBit::ShaderReadBit,
            .oldLayout = KDGpu::TextureLayout::TransferDstOptimal,
            .newLayout = KDGpu::TextureLayout::ShaderReadOnlyOptimal,
            .texture = texture,
    });
    //! [texture_upload]

    //! [texture_render_target]
    KDGpu::Texture renderTarget = device.createTexture(KDGpu::TextureOptions{
            .type = KDGpu::TextureType::TextureType2D,
            .format = KDGpu::Format::R8G8B8A8_UNORM,
            .extent = { 1920, 1080, 1 },
            .usage = KDGpu::TextureUsageFlagBits::ColorAttachmentBit | KDGpu::TextureUsageFlagBits::SampledBit,
    });

    KDGpu::TextureView renderView = renderTarget.createView();

    auto renderPass = recorder.beginRenderPass(KDGpu::RenderPassCommandRecorderOptions{
            .colorAttachments = {
                    {
                            .view = renderView,
                            .loadOperation = KDGpu::AttachmentLoadOperation::Clear,
                    },
            },
    });
    //! [texture_render_target]

    //! [texture_depth_buffer]
    uint32_t width = 1920, height = 1080;
    KDGpu::Texture depthBuffer = device.createTexture(KDGpu::TextureOptions{
            .type = KDGpu::TextureType::TextureType2D,
            .format = KDGpu::Format::D32_SFLOAT,
            .extent = { width, height, 1 },
            .usage = KDGpu::TextureUsageFlagBits::DepthStencilAttachmentBit,
    });
    //! [texture_depth_buffer]

    //! [texture_multisampled]
    KDGpu::Texture msaaTarget = device.createTexture(KDGpu::TextureOptions{
            .type = KDGpu::TextureType::TextureType2D,
            .format = KDGpu::Format::R8G8B8A8_SRGB,
            .extent = { 1920, 1080, 1 },
            .samples = KDGpu::SampleCountFlagBits::Samples4Bit,
            .usage = KDGpu::TextureUsageFlagBits::ColorAttachmentBit,
    });
    //! [texture_multisampled]

    KDGpu::Texture resolveTarget;

    //! [texture_resolve]
    auto resolveRenderPass = recorder.beginRenderPass(KDGpu::RenderPassCommandRecorderOptions{
            .colorAttachments = {
                    KDGpu::ColorAttachment{
                            .view = msaaTarget.createView(),
                            .resolveView = resolveTarget.createView(),
                            .loadOperation = KDGpu::AttachmentLoadOperation::Load,
                            .storeOperation = KDGpu::AttachmentStoreOperation::DontCare,
                    },
            },
    }); // MSAA target not needed after resolve
    //! [texture_resolve]

    //! [texture_storage_image]
    KDGpu::Texture storageTexture = device.createTexture(KDGpu::TextureOptions{
            .type = KDGpu::TextureType::TextureType2D,
            .format = KDGpu::Format::R32G32B32A32_SFLOAT,
            .extent = { 1024, 1024, 1 },
            .usage = KDGpu::TextureUsageFlagBits::StorageBit | KDGpu::TextureUsageFlagBits::SampledBit,
    });

    // Write to texture in compute shader
    // Read from texture in fragment shader
    //! [texture_storage_image]
}

// =============================================================================
// Handle Ownership Documentation Examples (kdgpu-handle-ownership.dox)
// =============================================================================

void handle_ownership_examples()
{
    // Note: These are conceptual examples from documentation.
    // They illustrate handle semantics and RAII patterns.

    //! [handle_ownership_basic_creation]
    // Handle is created and manages GPU resource lifetime
    // KDGpu::Buffer buffer = device.createBuffer(options);
    // Resource exists until 'buffer' goes out of scope
    //! [handle_ownership_basic_creation]

    //! [handle_ownership_move_semantics]
    // Handles support move semantics (no copying)
    // KDGpu::Buffer buffer1 = device.createBuffer(options);
    // KDGpu::Buffer buffer2 = std::move(buffer1); // Ownership transferred
    // buffer1 is now invalid, buffer2 owns the resource
    //! [handle_ownership_move_semantics]

    //! [handle_ownership_no_copy]
    // Handles cannot be copied (deleted copy constructor/assignment)
    // KDGpu::Buffer buffer1 = device.createBuffer(options);
    // KDGpu::Buffer buffer2 = buffer1; // ERROR: Won't compile
    //! [handle_ownership_no_copy]

    //! [handle_ownership_destruction]
    // Resource automatically destroyed when handle goes out of scope
    // {
    //     KDGpu::Buffer buffer = device.createBuffer(options);
    //     // ... use buffer ...
    // } // buffer destroyed here, GPU memory freed
    //! [handle_ownership_destruction]

    //! [handle_ownership_explicit_release]
    // Explicit resource release before destruction
    // KDGpu::Buffer buffer = device.createBuffer(options);
    // // ... use buffer ...
    // buffer = {}; // Explicitly destroy resource now
    //! [handle_ownership_explicit_release]

    //! [handle_ownership_container]
    // Handles can be stored in containers
    // std::vector<KDGpu::Buffer> buffers;
    // buffers.push_back(device.createBuffer(options1));
    // buffers.push_back(device.createBuffer(options2));
    // All resources freed when vector destroyed
    //! [handle_ownership_container]

    //! [handle_ownership_member_variable]
    // Handles as class members
    // class Mesh {
    //   KDGpu::Buffer m_vertexBuffer;
    //   KDGpu::Buffer m_indexBuffer;
    // public:
    //   Mesh(KDGpu::Device& device) {
    //     m_vertexBuffer = device.createBuffer(vertexOptions);
    //     m_indexBuffer = device.createBuffer(indexOptions);
    //   }
    //   // Buffers automatically destroyed with Mesh
    // };
    //! [handle_ownership_member_variable]

    //! [handle_ownership_factory_function]
    // Returning handles from functions
    // KDGpu::Buffer createVertexBuffer(KDGpu::Device& device) {
    //   return device.createBuffer(options); // Move semantics
    // }
    // KDGpu::Buffer buffer = createVertexBuffer(device);
    //! [handle_ownership_factory_function]

    //! [handle_ownership_optional]
    // Optional handles for conditional resources
    // std::optional<KDGpu::Texture> depthTexture;
    // if (needsDepth) {
    //   depthTexture = device.createTexture(options);
    // }
    //! [handle_ownership_optional]

    //! [handle_ownership_shared_ownership]
    // Shared ownership via std::shared_ptr if needed
    // auto buffer = std::make_shared<KDGpu::Buffer>(device.createBuffer(options));
    // std::shared_ptr<KDGpu::Buffer> sharedRef = buffer;
    // Resource freed when all shared_ptrs destroyed
    //! [handle_ownership_shared_ownership]

    //! [handle_ownership_device_lifetime]
    // Device must outlive all resources created from it
    // Device lifetime typically managed at application scope
    //! [handle_ownership_device_lifetime]

    //! [handle_ownership_command_buffers]
    // Command buffers follow same ownership rules
    // KDGpu::CommandRecorder recorder = device.createCommandRecorder();
    // // ... record commands ...
    // KDGpu::CommandBuffer cmdBuffer = recorder.finish();
    // queue.submit(SubmitOptions{ .commandBuffers = { cmdBuffer } });
    // // cmdBuffer can be destroyed after submit() returns
    //! [handle_ownership_command_buffers]

    //! [handle_ownership_views]
    // Views reference parent resource - parent must outlive views
    // KDGpu::Texture texture = device.createTexture(options);
    // KDGpu::TextureView view = texture.createView();
    // // view is valid while texture exists
    // // Destroying texture invalidates view
    //! [handle_ownership_views]

    //! [handle_ownership_bind_groups]
    // Bind groups reference resources - don't destroy resources while in use
    // KDGpu::Buffer buffer = device.createBuffer(options);
    // KDGpu::BindGroup bindGroup = device.createBindGroup(BindGroupOptions{
    //   .resources = {{ .resource = UniformBufferBinding{ .buffer = buffer.handle() } }}
    // });
    // // buffer must remain valid while bindGroup is used
    //! [handle_ownership_bind_groups]

    //! [handle_ownership_pipeline_layout]
    // Pipeline layouts reference bind group layouts
    // KDGpu::BindGroupLayout bgl = device.createBindGroupLayout(options);
    // KDGpu::PipelineLayout layout = device.createPipelineLayout(PipelineLayoutOptions{
    //   .bindGroupLayouts = { bgl.handle() }
    // });
    // // bgl must outlive layout
    //! [handle_ownership_pipeline_layout]

    //! [handle_ownership_synchronization]
    // Fences and semaphores follow same RAII pattern
    // KDGpu::Fence fence = device.createFence();
    // queue.submit(SubmitOptions{ .signalFence = fence });
    // fence.wait();
    // // fence destroyed when out of scope
    //! [handle_ownership_synchronization]

    //! [handle_ownership_pools]
    // Pools allocate resources with shorter lifetimes
    // KDGpu::BindGroupPool pool = device.createBindGroupPool(options);
    // KDGpu::BindGroup bg = pool.allocateBindGroup(layout);
    // pool.reset(); // Frees all allocated bind groups
    //! [handle_ownership_pools]

    //! [handle_ownership_best_practices]
    // Best practices:
    // - Prefer stack allocation over heap when possible
    // - Use RAII - let handles manage lifetimes automatically
    // - Ensure parent resources outlive dependent resources
    // - Use move semantics to transfer ownership
    // - Avoid manual resource management (no need for destroy())
    //! [handle_ownership_best_practices]
}

// =============================================================================
// API Overview Documentation Examples (kdgpu-api-overview.dox)
// =============================================================================

void api_overview_examples()
{
    //! [api_overview_basic_structure]
    // KDGpu API consists of:
    // 1. Instance - entry point, enumerates adapters
    // 2. Adapter - represents physical GPU
    // 3. Device - logical GPU handle, creates resources
    // 4. Resources - buffers, textures, pipelines, etc.
    // 5. Command recording - record GPU work
    // 6. Synchronization - fences, semaphores for CPU-GPU and GPU-GPU sync
    //! [api_overview_basic_structure]

    //! [api_overview_initialization]
    // Typical initialization flow:
    KDGpu::VulkanGraphicsApi vulkanGraphisApi;
    KDGpu::Instance instance = vulkanGraphisApi.createInstance(KDGpu::InstanceOptions{ /* ... */ });
    KDGpu::Adapter *adapter = instance.selectAdapter(KDGpu::AdapterDeviceType::Default); // Select GPU
    KDGpu::Device device = adapter->createDevice(KDGpu::DeviceOptions{
            .requestedFeatures = adapter->features(),
    });
    // Device ready to create resources and submit work
    KDGpu::Buffer buffer = device.createBuffer(KDGpu::BufferOptions{
            .usage = KDGpu::BufferUsageFlagBits::UniformBufferBit,
            .memoryUsage = KDGpu::MemoryUsage::GpuOnly,
    });
    //! [api_overview_initialization]
}

// =============================================================================
// Handles Documentation Examples (kdgpu-handle-ownership.dox)
// =============================================================================
void explicit_vs_impplicit_handles()
{
    KDGpu::Device device;
    //! [explicit_vs_impplicit_handles]
    KDGpu::Texture texture = device.createTexture(KDGpu::TextureOptions{
            /* ... */
    });
    KDGpu::TextureView view = texture.createView(KDGpu::TextureViewOptions{
            /* ... */
    });

    KDGpu::Sampler sampler = device.createSampler(KDGpu::SamplerOptions{
            /* ... */
    });
    KDGpu::BindGroupLayout bindGroupLayout = device.createBindGroupLayout(KDGpu::BindGroupLayoutOptions{
            /* .... */
    });
    // Explicit handle extraction (verbose)
    KDGpu::BindGroup bindGroup = device.createBindGroup(KDGpu::BindGroupOptions{
            .layout = bindGroupLayout.handle(), // Explicit .handle() call
            .resources = {
                    {
                            .binding = 0,
                            .resource = KDGpu::TextureViewSamplerBinding{
                                    .textureView = view.handle(), // Explicit .handle() call
                                    .sampler = sampler.handle(), // Explicit .handle() call
                            },
                    },
            },
    });

    // Implicit conversion (cleaner)
    // Works with any API expecting a handle
    KDGpu::BindGroup bindGroup2 = device.createBindGroup(KDGpu::BindGroupOptions{
            .layout = bindGroupLayout, // Implicitly converts to Handle<BindGroupLayout_t>
            .resources = {
                    {
                            .binding = 0,
                            .resource = KDGpu::TextureViewSamplerBinding{
                                    .textureView = view, // Implicitly converts to Handle<TextureView_t>
                                    .sampler = sampler, // Implicitly converts to Handle<Sampler_t>
                            },
                    },
            },
    });
    //! [explicit_vs_impplicit_handles]
}

void handle_is_valid()
{
    //! [handle_is_valid]
    KDGpu::Buffer buffer;
    if (buffer.isValid()) {
        // Safe to use
        void *ptr = buffer.map();
    }

    // Or check the handle directly
    if (buffer.handle().isValid()) {
        // Handle is valid
    }
    //! [handle_is_valid]
}

void access_underlying_vulkan_resource()
{
    KDGpu::Device device;
    //! [access_underlying_vulkan_resource]
    KDGpu::Buffer buffer = device.createBuffer(KDGpu::BufferOptions{
            /* ... */
    });
    KDGpu::Handle<KDGpu::Buffer_t> kdgpuHandle = buffer.handle();

    // Access underlying VulkanBuffer
    KDGpu::GraphicsApi *api = device.graphicsApi();
    KDGpu::VulkanResourceManager *rm = api->resourceManager();
    KDGpu::VulkanBuffer *vkBuffer = rm->getBuffer(kdgpuHandle);

    // Access raw VkBuffer
    VkBuffer rawVkBuffer = vkBuffer->buffer; // Use with care!
    //! [access_underlying_vulkan_resource]
}

// =============================================================================
// Sampler Documentation Examples (sampler.h)
// =============================================================================

void sampler_examples(KDGpu::Device &device)
{
    KDGpu::TextureView textureView;
    KDGpu::BindGroupLayout layout;

    //! [sampler_basic]
    KDGpu::Sampler sampler = device.createSampler(KDGpu::SamplerOptions{
            .magFilter = KDGpu::FilterMode::Linear,
            .minFilter = KDGpu::FilterMode::Linear,
            .mipmapFilter = KDGpu::MipmapFilterMode::Linear,
            .u = KDGpu::AddressMode::Repeat,
            .v = KDGpu::AddressMode::Repeat,
            .w = KDGpu::AddressMode::Repeat,
    });

    // Use with texture in bind group
    KDGpu::BindGroup bindGroup = device.createBindGroup(KDGpu::BindGroupOptions{
            .layout = layout.handle(),
            .resources = {
                    {
                            .binding = 0,
                            .resource = KDGpu::TextureViewSamplerBinding{
                                    .textureView = textureView,
                                    .sampler = sampler,
                            },
                    },
            },
    });
    //! [sampler_basic]

    //! [sampler_linear]
    KDGpu::Sampler linearSampler = device.createSampler(KDGpu::SamplerOptions{
            .magFilter = KDGpu::FilterMode::Linear,
            .minFilter = KDGpu::FilterMode::Linear,
            .u = KDGpu::AddressMode::ClampToEdge,
            .v = KDGpu::AddressMode::ClampToEdge,
    });
    //! [sampler_linear]

    //! [sampler_nearest]
    KDGpu::Sampler pixelSampler = device.createSampler(KDGpu::SamplerOptions{
            .magFilter = KDGpu::FilterMode::Nearest, // No interpolation
            .minFilter = KDGpu::FilterMode::Nearest,
            .u = KDGpu::AddressMode::ClampToEdge,
            .v = KDGpu::AddressMode::ClampToEdge,
    });
    //! [sampler_nearest]

    //! [sampler_anisotropic]
    KDGpu::Sampler anisotropicSampler = device.createSampler(KDGpu::SamplerOptions{
            .magFilter = KDGpu::FilterMode::Linear,
            .minFilter = KDGpu::FilterMode::Linear,
            .mipmapFilter = KDGpu::MipmapFilterMode::Linear,
            .u = KDGpu::AddressMode::Repeat,
            .v = KDGpu::AddressMode::Repeat,
            .anisotropyEnabled = true,
            .maxAnisotropy = 16.0f,
    }); // Check adapter limits
    //! [sampler_anisotropic]

    //! [sampler_edge]
    KDGpu::Sampler edgeSampler = device.createSampler(KDGpu::SamplerOptions{
            .magFilter = KDGpu::FilterMode::Linear,
            .minFilter = KDGpu::FilterMode::Linear,
            .u = KDGpu::AddressMode::ClampToEdge,
            .v = KDGpu::AddressMode::ClampToEdge,
    });
    //! [sampler_edge]

    //! [sampler_border]
    KDGpu::Sampler borderSampler = device.createSampler(KDGpu::SamplerOptions{
            .magFilter = KDGpu::FilterMode::Linear,
            .minFilter = KDGpu::FilterMode::Linear,
            .u = KDGpu::AddressMode::ClampToBorder,
            .v = KDGpu::AddressMode::ClampToBorder,
    });
    //! [sampler_border]

    //! [sampler_lod]
    KDGpu::Sampler lodSampler = device.createSampler(KDGpu::SamplerOptions{
            .magFilter = KDGpu::FilterMode::Linear,
            .minFilter = KDGpu::FilterMode::Linear,
            .mipmapFilter = KDGpu::MipmapFilterMode::Linear,
            .u = KDGpu::AddressMode::Repeat,
            .v = KDGpu::AddressMode::Repeat,
            .lodMinClamp = 0.0f, // Finest mip level
            .lodMaxClamp = 10.0f,
    }); // Coarsest mip level
    //! [sampler_lod]

    KDGpu::SamplerOptions linearRepeatOptions{
        .magFilter = KDGpu::FilterMode::Linear,
        .minFilter = KDGpu::FilterMode::Linear,
        .u = KDGpu::AddressMode::Repeat,
        .v = KDGpu::AddressMode::Repeat
    };
    KDGpu::TextureView albedoView, normalView;

    //! [sampler_reuse]
    // Create once, use many times
    KDGpu::Sampler linearRepeat = device.createSampler(linearRepeatOptions);

    // Use with multiple textures
    KDGpu::BindGroup bindGroup1 = device.createBindGroup({
            .layout = layout.handle(),
            .resources = {
                    {
                            .binding = 0,
                            .resource = KDGpu::TextureViewSamplerBinding{
                                    .textureView = albedoView.handle(),
                                    .sampler = linearRepeat.handle() // Shared sampler
                            },
                    },
            },
    });

    KDGpu::BindGroup bindGroup2 = device.createBindGroup({
            .layout = layout.handle(),
            .resources = {
                    {
                            .binding = 0,
                            .resource = KDGpu::TextureViewSamplerBinding{
                                    .textureView = normalView.handle(),
                                    .sampler = linearRepeat.handle() // Same sampler
                            },
                    },
            },
    });
    //! [sampler_reuse]
}

// =============================================================================
// Swapchain Documentation Examples (swapchain.h)
// =============================================================================

void swapchain_examples(KDGpu::Instance &instance, KDGpu::Device &device, KDGpu::Queue &queue)
{
    KDGpu::SurfaceOptions surfaceOptions;
    uint32_t windowWidth = 1920, windowHeight = 1080;

    //! [swapchain_creation]
    KDGpu::Surface surface = instance.createSurface(surfaceOptions);
    KDGpu::Adapter *adapter = instance.selectAdapter(KDGpu::AdapterDeviceType::Default);

    // Query surface capabilities
    auto swapchainProps = adapter->swapchainProperties(surface);

    KDGpu::Swapchain swapchain = device.createSwapchain(KDGpu::SwapchainOptions{
            .surface = surface,
            .format = swapchainProps.formats[0].format, // Choose first available
            .minImageCount = 3, // Triple buffering
            .imageExtent = { windowWidth, windowHeight },
            .imageUsageFlags = KDGpu::TextureUsageFlagBits::ColorAttachmentBit,
    });
    //! [swapchain_creation]

    //! [swapchain_images]
    const std::vector<KDGpu::Texture> &images = swapchain.textures();

    // Create views for rendering
    std::vector<KDGpu::TextureView> imageViews;
    for (const auto &image : images) {
        imageViews.push_back(image.createView());
    }
    //! [swapchain_images]

    bool rendering = true;

    //! [swapchain_render_loop]
    KDGpu::GpuSemaphore imageAvailable = device.createGpuSemaphore();
    KDGpu::GpuSemaphore renderComplete = device.createGpuSemaphore();
    KDGpu::CommandRecorder recorder = device.createCommandRecorder();

    while (rendering) {
        // Acquire next image to render to
        uint32_t imageIndex;
        const KDGpu::AcquireImageResult result = swapchain.getNextImageIndex(imageIndex, imageAvailable);

        if (result != KDGpu::AcquireImageResult::Success) {
            // Handle swapchain out of date (window resize, etc.)
            continue;
        }

        // Record rendering commands to swapchain image
        auto renderPass = recorder.beginRenderPass(KDGpu::RenderPassCommandRecorderOptions{
                .colorAttachments = {
                        {
                                .view = imageViews[imageIndex],
                                .loadOperation = KDGpu::AttachmentLoadOperation::Clear,
                                .storeOperation = KDGpu::AttachmentStoreOperation::Store,
                                .clearValue = { 0.1f, 0.1f, 0.1f, 1.0f },
                        },
                },
        });
        // ... render commands ...
        renderPass.end();
        KDGpu::CommandBuffer cmdBuffer = recorder.finish();

        // Submit rendering
        queue.submit(KDGpu::SubmitOptions{
                .commandBuffers = { cmdBuffer },
                .waitSemaphores = { imageAvailable.handle() },
                .signalSemaphores = { renderComplete.handle() },
        });

        // Present the rendered image
        queue.present(KDGpu::PresentOptions{
                .waitSemaphores = { renderComplete.handle() },
                .swapchainInfos = {
                        {
                                .swapchain = swapchain.handle(),
                                .imageIndex = imageIndex,
                        },
                },
        });
    }
    //! [swapchain_render_loop]

    uint32_t newWidth = 1920, newHeight = 1080;
    KDGpu::Format surfaceFormat = swapchainProps.formats[0].format;

    //! [swapchain_resize]
    // Wait for GPU to finish using swapchain
    device.waitUntilIdle();

    // Destroy old image views
    imageViews.clear();

    // Recreate swapchain with new size
    swapchain = device.createSwapchain(KDGpu::SwapchainOptions{
            .surface = surface,
            .format = surfaceFormat,
            .minImageCount = 3,
            .imageExtent = { newWidth, newHeight },
            .imageUsageFlags = KDGpu::TextureUsageFlagBits::ColorAttachmentBit,
    });

    // Recreate views
    for (const KDGpu::Texture &image : swapchain.textures()) {
        imageViews.push_back(image.createView());
    }
    //! [swapchain_resize]

    //! [swapchain_present_mode]
    // Find best presentation mode
    KDGpu::PresentMode presentMode = KDGpu::PresentMode::Fifo; // VSync (always available)

    for (const auto &mode : swapchainProps.presentModes) {
        if (mode == KDGpu::PresentMode::Mailbox) {
            // Triple buffering without tearing
            presentMode = mode;
            break;
        } else if (mode == KDGpu::PresentMode::Immediate) {
            // No VSync, potential tearing
            presentMode = mode;
        }
    }

    KDGpu::Swapchain swapchainWithMode = device.createSwapchain(KDGpu::SwapchainOptions{
            .surface = surface.handle(),
            .format = surfaceFormat,
            .imageExtent = { newWidth, newHeight },
            .presentMode = presentMode,
    });
    //! [swapchain_present_mode]

    //! [swapchain_format]
    auto props = adapter->swapchainProperties(surface.handle());

    // Find SRGB format if available
    KDGpu::Format chosenFormat = props.formats[0].format;
    for (const auto &formatInfo : props.formats) {
        if (formatInfo.format == KDGpu::Format::B8G8R8A8_SRGB) {
            chosenFormat = formatInfo.format;
            break;
        }
    }
    //! [swapchain_format]
}

// =============================================================================
// Surface Documentation Examples (surface.h)
// =============================================================================

void surface_examples(KDGpu::Instance &instance)
{
    KDGpu::SurfaceOptions surfaceOptions;

    //! [surface_creation]
    // Note: Platform-specific - actual implementation requires native window handles
    // See platform-specific documentation for surface creation
    //! [surface_creation]

    KDGpu::Surface surface = instance.createSurface(surfaceOptions);

    //! [surface_adapter_support]
    KDGpu::Adapter *adapter = instance.selectAdapter(KDGpu::AdapterDeviceType::DiscreteGpu);

    // Check if adapter supports presenting to this surface
    auto queueTypes = adapter->queueTypes();
    for (size_t i = 0; i < queueTypes.size(); ++i) {
        if (adapter->supportsPresentation(surface, i)) {
            // Queue family i can present to this surface
        }
    }
    //! [surface_adapter_support]

    //! [surface_capabilities]
    auto swapchainProps = adapter->swapchainProperties(surface.handle());

    // Check supported formats
    for (const auto &format : swapchainProps.formats) {
        if (format.format == KDGpu::Format::B8G8R8A8_SRGB) {
            // Surface supports SRGB format
        }
    }

    // Get surface size limits
    uint32_t minWidth = swapchainProps.capabilities.minImageExtent.width;
    uint32_t maxWidth = swapchainProps.capabilities.maxImageExtent.width;
    uint32_t minImages = swapchainProps.capabilities.minImageCount;
    uint32_t maxImages = swapchainProps.capabilities.maxImageCount;
    //! [surface_capabilities]

    //! [surface_swapchain]
    KDGpu::Device device = adapter->createDevice();

    // Create swapchain for presenting to surface
    uint32_t windowWidth = 1920, windowHeight = 1080;
    KDGpu::Swapchain swapchain = device.createSwapchain(KDGpu::SwapchainOptions{
            .surface = surface.handle(),
            .minImageCount = 3, // Triple buffering
            .imageExtent = { windowWidth, windowHeight },
            .imageUsageFlags = KDGpu::TextureUsageFlagBits::ColorAttachmentBit,
    });
    //! [surface_swapchain]

    KDGpu::InstanceOptions instanceOptions;
    KDGpu::GraphicsApi *api = nullptr;
    uint32_t width = 1920, height = 1080;

    //! [surface_complete_integration]
    // Create instance
    KDGpu::Instance instanceComplete = api->createInstance(instanceOptions);

    // Create surface from window
    KDGpu::Surface surfaceComplete = instanceComplete.createSurface(surfaceOptions);

    // Select adapter that supports the surface
    KDGpu::Adapter *adapterComplete = instanceComplete.selectAdapter(KDGpu::AdapterDeviceType::DiscreteGpu);

    // Query supported formats
    auto swapchainPropsComplete = adapterComplete->swapchainProperties(surfaceComplete.handle());
    KDGpu::Format surfaceFormat = swapchainPropsComplete.formats[0].format;

    // Create device
    KDGpu::Device deviceComplete = adapterComplete->createDevice();

    // Create swapchain
    KDGpu::Swapchain swapchainComplete = deviceComplete.createSwapchain(KDGpu::SwapchainOptions{
            .surface = surfaceComplete.handle(),
            .format = surfaceFormat,
            .minImageCount = 3,
            .imageExtent = { width, height },
            .imageUsageFlags = KDGpu::TextureUsageFlagBits::ColorAttachmentBit,
    });
    //! [surface_complete_integration]
}

// =============================================================================
// Additional Texture Examples
// =============================================================================

void texture_view_examples(KDGpu::Texture &texture, KDGpu::Texture &cubemap)
{
    //! [texture_views]
    // Full texture view
    KDGpu::TextureView fullView = texture.createView();

    // Single mip level view
    KDGpu::TextureView mipView = texture.createView(KDGpu::TextureViewOptions{
            .range = {
                    .aspectMask = KDGpu::TextureAspectFlagBits::ColorBit,
                    .baseMipLevel = 2,
                    .levelCount = 1,
            },
    });

    // Cube map face view
    KDGpu::TextureView faceView = cubemap.createView(KDGpu::TextureViewOptions{
            .viewType = KDGpu::ViewType::ViewType2D,
            .range = {
                    .aspectMask = KDGpu::TextureAspectFlagBits::ColorBit,
                    .baseArrayLayer = 0, // First face
                    .layerCount = 1,
            },
    });
    //! [texture_views]
}

void texture_format_examples(KDGpu::Device &device)
{
    //! [texture_formats]
    // Color formats
    KDGpu::Format rgba8 = KDGpu::Format::R8G8B8A8_UNORM; // Standard 8-bit RGBA
    KDGpu::Format rgba8srgb = KDGpu::Format::R8G8B8A8_SRGB; // SRGB color space
    KDGpu::Format rgba16f = KDGpu::Format::R16G16B16A16_SFLOAT; // HDR (half float)
    KDGpu::Format rgba32f = KDGpu::Format::R32G32B32A32_SFLOAT; // HDR (full float)
    KDGpu::Format bgra8srgb = KDGpu::Format::B8G8R8A8_SRGB; // Common swapchain format

    // Depth formats
    KDGpu::Format depth32 = KDGpu::Format::D32_SFLOAT; // 32-bit depth
    KDGpu::Format depth24s8 = KDGpu::Format::D24_UNORM_S8_UINT; // 24-bit depth + 8-bit stencil
    KDGpu::Format depth16 = KDGpu::Format::D16_UNORM; // 16-bit depth

    // Compressed formats
    KDGpu::Format bc1 = KDGpu::Format::BC1_RGB_SRGB_BLOCK; // DXT1
    KDGpu::Format bc3 = KDGpu::Format::BC3_SRGB_BLOCK; // DXT5
    //! [texture_formats]
}

// =============================================================================
// Additional Shader Module Examples
// =============================================================================

void shadermodule_reuse_examples(KDGpu::Device &device)
{
    std::vector<uint32_t> vertSpirv = loadSpirv("shader.vert.spv");
    std::vector<uint32_t> fragSpirv = loadSpirv("shader.frag.spv");

    //! [shadermodule_reuse]
    KDGpu::ShaderModule vertShader = device.createShaderModule(vertSpirv);
    KDGpu::ShaderModule fragShader = device.createShaderModule(fragSpirv);

    // Use same shaders for multiple pipelines with different state
    KDGpu::PipelineLayout pipelineLayout;
    KDGpu::Format swapchainFormat = KDGpu::Format::B8G8R8A8_SRGB;

    KDGpu::GraphicsPipeline opaquePipeline = device.createGraphicsPipeline(KDGpu::GraphicsPipelineOptions{
            .shaderStages = {
                    {
                            .shaderModule = vertShader,
                            .stage = KDGpu::ShaderStageFlagBits::VertexBit,
                    },
                    {
                            .shaderModule = fragShader,
                            .stage = KDGpu::ShaderStageFlagBits::FragmentBit,
                    },
            },
            .layout = pipelineLayout,
            .renderTargets = {
                    {
                            .format = swapchainFormat,
                    },
            },
    });

    KDGpu::GraphicsPipeline transparentPipeline = device.createGraphicsPipeline(KDGpu::GraphicsPipelineOptions{
            .shaderStages = {
                    {
                            .shaderModule = vertShader,
                            .stage = KDGpu::ShaderStageFlagBits::VertexBit,
                    },
                    {
                            .shaderModule = fragShader,
                            .stage = KDGpu::ShaderStageFlagBits::FragmentBit,
                    },
            },
            .layout = pipelineLayout,
            .renderTargets = {
                    {
                            .format = swapchainFormat,
                    },
            },
    });
    //! [shadermodule_reuse]

    std::vector<uint32_t> geomSpirv = loadSpirv("shader.geom.spv");

    //! [shadermodule_geometry]
    KDGpu::ShaderModule geomShader = device.createShaderModule(geomSpirv);

    KDGpu::GraphicsPipeline pipeline = device.createGraphicsPipeline(KDGpu::GraphicsPipelineOptions{
            .shaderStages = {
                    {
                            .shaderModule = vertShader,
                            .stage = KDGpu::ShaderStageFlagBits::VertexBit,
                    },
                    {
                            .shaderModule = geomShader,
                            .stage = KDGpu::ShaderStageFlagBits::GeometryBit,
                    },
                    {
                            .shaderModule = fragShader,
                            .stage = KDGpu::ShaderStageFlagBits::FragmentBit,
                    },
            },
            .layout = pipelineLayout,
            .renderTargets = {
                    {
                            .format = swapchainFormat,
                    },
            },
    });
    //! [shadermodule_geometry]
}

// =============================================================================
// Additional Adapter Examples
// =============================================================================

void adapter_member_examples(KDGpu::Adapter *adapter, KDGpu::Surface surface)
{
    //! [adapter_extensions]
    auto extensions = adapter->extensions();
    for (const auto &ext : extensions) {
        if (ext.name == "VK_KHR_ray_tracing_pipeline") {
            std::cout << "Ray tracing supported!" << std::endl;
        }
    }
    //! [adapter_extensions]

    //! [adapter_presentation_support]
    for (uint32_t i = 0; i < adapter->queueTypes().size(); ++i) {
        if (adapter->supportsPresentation(surface.handle(), i)) {
            std::cout << "Queue family " << i << " can present" << std::endl;
        }
    }
    //! [adapter_presentation_support]

    //! [adapter_blitting]
    if (adapter->supportsBlitting(
                KDGpu::Format::R8G8B8A8_UNORM, KDGpu::TextureTiling::Optimal,
                KDGpu::Format::R8G8B8A8_SRGB, KDGpu::TextureTiling::Optimal)) {
        // Can blit from UNORM to SRGB
    }
    //! [adapter_blitting]
}

void renderpass_command_recorder_examples(KDGpu::Device *device, KDGpu::CommandRecorder *commandRecorder,
                                          KDGpu::TextureView colorView, KDGpu::TextureView msaaColorView,
                                          KDGpu::TextureView arrayColorView, KDGpu::TextureView depthView,
                                          KDGpu::GraphicsPipeline pipeline, KDGpu::BindGroup bindGroup,
                                          KDGpu::Buffer vertexBuffer, KDGpu::Buffer indexBuffer)
{
    {
        //! [renderpass_dynamic_rendering_basic]
        // Using dynamic rendering (modern approach, promoted to Vulkan 1.3 core)
        KDGpu::RenderPassCommandRecorderWithDynamicRenderingOptions renderOptions{
            // Configure color attachment (e.g., swapchain image)
            .colorAttachments = {
                    KDGpu::ColorAttachment{
                            .view = colorView.handle(),
                            .loadOperation = KDGpu::AttachmentLoadOperation::Clear,
                            .storeOperation = KDGpu::AttachmentStoreOperation::Store,
                            .clearValue = { 0.1f, 0.2f, 0.3f, 1.0f }, // Clear to dark blue
                            .initialLayout = KDGpu::TextureLayout::Undefined,
                            .layout = KDGpu::TextureLayout::ColorAttachmentOptimal,
                            .finalLayout = KDGpu::TextureLayout::PresentSrc,
                    },
            },
            // Configure depth attachment
            .depthStencilAttachment = KDGpu::DepthStencilAttachment{
                    .view = depthView,
                    .depthLoadOperation = KDGpu::AttachmentLoadOperation::Clear,
                    .depthStoreOperation = KDGpu::AttachmentStoreOperation::DontCare,
                    .depthClearValue = 1.0f,
                    .initialLayout = KDGpu::TextureLayout::Undefined,
                    .layout = KDGpu::TextureLayout::DepthStencilAttachmentOptimal,
            },
        };

        // Begin render pass
        auto renderPass = commandRecorder->beginRenderPass(renderOptions);

        // Record rendering commands...
        renderPass.setPipeline(pipeline);
        renderPass.setVertexBuffer(0, vertexBuffer);
        renderPass.setIndexBuffer(indexBuffer);
        renderPass.setBindGroup(0, bindGroup);
        renderPass.drawIndexed({ .indexCount = 36 });

        // End the render pass
        renderPass.end();
        //! [renderpass_dynamic_rendering_basic]
    }

    {
        //! [renderpass_msaa_resolve]
        // Multi-sample anti-aliasing with automatic resolve

        KDGpu::RenderPassCommandRecorderWithDynamicRenderingOptions msaaOptions{
            .colorAttachments = {
                    KDGpu::ColorAttachment{
                            .view = msaaColorView, // MSAA texture view
                            .resolveView = colorView, // Single-sample texture to resolve to
                            .loadOperation = KDGpu::AttachmentLoadOperation::Clear,
                            .storeOperation = KDGpu::AttachmentStoreOperation::DontCare, // MSAA buffer not needed after resolve
                            .clearValue = { 0.0f, 0.0f, 0.0f, 1.0f },
                    },
            },
            .samples = KDGpu::SampleCountFlagBits::Samples4Bit,
        };

        auto msaaRenderPass = commandRecorder->beginRenderPass(msaaOptions);
        // Render with MSAA...
        msaaRenderPass.end();
        //! [renderpass_msaa_resolve]
    }

    {
        //! [renderpass_multiview]
        // Multi-view rendering for VR/stereo (renders to both eyes in a single pass)
        KDGpu::RenderPassCommandRecorderWithDynamicRenderingOptions multiviewOptions{
            .colorAttachments = {
                    KDGpu::ColorAttachment{
                            .view = arrayColorView, // Array texture with 2 layers
                            .loadOperation = KDGpu::AttachmentLoadOperation::Clear,
                            .storeOperation = KDGpu::AttachmentStoreOperation::Store,
                    },
            },
            .viewCount = 2, // Render to 2 views simultaneously
        };

        auto multiviewRenderPass = commandRecorder->beginRenderPass(multiviewOptions);
        // The vertex shader will run once per view using gl_ViewIndex
        multiviewRenderPass.end();
        //! [renderpass_multiview]
    }

    KDGpu::RenderPassCommandRecorderWithDynamicRenderingOptions renderOptions{};
    {
        //! [renderpass_viewport_scissor]
        auto renderPass = commandRecorder->beginRenderPass(renderOptions);

        // Set dynamic viewport
        KDGpu::Viewport viewport{
            .x = 0.0f,
            .y = 0.0f,
            .width = 1920.0f,
            .height = 1080.0f,
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        renderPass.setViewport(viewport);

        // Set scissor rectangle (can be smaller than viewport for clipping)
        KDGpu::Rect2D scissor{
            .offset = { 100, 100 },
            .extent = { 800, 600 },
        };
        renderPass.setScissor(scissor);

        renderPass.end();
        //! [renderpass_viewport_scissor]
    }

    {
        //! [renderpass_draw_variants]
        auto renderPass = commandRecorder->beginRenderPass(renderOptions);
        renderPass.setPipeline(pipeline);

        // Simple draw (non-indexed)
        KDGpu::DrawCommand drawCmd{
            .vertexCount = 3, // 3 vertices for a triangle
            .instanceCount = 1,
            .firstVertex = 0,
            .firstInstance = 0,
        };
        renderPass.draw(drawCmd);

        // Indexed draw (most common)
        KDGpu::DrawIndexedCommand indexedCmd{
            .indexCount = 36, // 36 indices for a cube
            .instanceCount = 1,
            .firstIndex = 0,
            .vertexOffset = 0,
            .firstInstance = 0,
        };
        renderPass.drawIndexed(indexedCmd);

        // Instanced rendering
        KDGpu::DrawIndexedCommand instancedCmd{
            .indexCount = 36,
            .instanceCount = 100, // Draw 100 instances
            .firstIndex = 0,
            .vertexOffset = 0,
            .firstInstance = 0,
        };
        renderPass.drawIndexed(instancedCmd);

        renderPass.end();
        //! [renderpass_draw_variants]
    }

    {
        //! [renderpass_push_constants]
        auto renderPass = commandRecorder->beginRenderPass(renderOptions);
        renderPass.setPipeline(pipeline);

        // Upload small amounts of data directly to shaders (faster than UBO for small data)
        struct PushConstants {
            glm::mat4 transform;
            glm::vec4 color;
        } pushData;

        pushData.transform = glm::mat4(1.0f);
        pushData.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

        KDGpu::PushConstantRange range{
            .offset = 0,
            .size = sizeof(PushConstants),
            .shaderStages = KDGpu::ShaderStageFlagBits::VertexBit | KDGpu::ShaderStageFlagBits::FragmentBit,
        };

        renderPass.pushConstant(range, &pushData);
        renderPass.drawIndexed({ .indexCount = 36 });

        renderPass.end();
        //! [renderpass_push_constants]
    }

    {
        //! [renderpass_stencil_reference]
        auto renderPass = commandRecorder->beginRenderPass(renderOptions);
        renderPass.setPipeline(pipeline.handle());

        // Set stencil reference value dynamically (for stencil masking/testing)
        renderPass.setStencilReference(
                KDGpu::StencilFaceFlags(KDGpu::StencilFaceFlagBits::FrontAndBack),
                42 // Reference value
        );

        renderPass.end();
        //! [renderpass_stencil_reference]
    }

    {
        //! [renderpass_multiple_attachments]
        // Multiple render targets (MRT) for deferred rendering
        KDGpu::RenderPassCommandRecorderWithDynamicRenderingOptions mrtOptions{
            .colorAttachments = {
                    // Attachment 0: Albedo/diffuse color
                    KDGpu::ColorAttachment{
                            .view = colorView,
                            .loadOperation = KDGpu::AttachmentLoadOperation::Clear,
                            .storeOperation = KDGpu::AttachmentStoreOperation::Store,
                    },

                    // Attachment 1: Normals
                    KDGpu::ColorAttachment{
                            .view = colorView,
                            .loadOperation = KDGpu::AttachmentLoadOperation::Clear,
                            .storeOperation = KDGpu::AttachmentStoreOperation::Store,
                    },

                    // Attachment 2: Material properties (roughness, metallic, etc.)
                    KDGpu::ColorAttachment{
                            .view = colorView,
                            .loadOperation = KDGpu::AttachmentLoadOperation::Clear,
                            .storeOperation = KDGpu::AttachmentStoreOperation::Store,
                    },
            },
        };

        auto mrtRenderPass = commandRecorder->beginRenderPass(mrtOptions);
        // Fragment shader outputs to multiple render targets
        mrtRenderPass.end();
        //! [renderpass_multiple_attachments]
    }

    {
        //! [renderpass_load_operations]
        KDGpu::RenderPassCommandRecorderWithDynamicRenderingOptions loadOptions{};

        KDGpu::ColorAttachment attachment{
            .view = colorView,
            // Clear: Discard previous contents, initialize with clearValue
            // Use when you'll draw to the entire attachment
            .loadOperation = KDGpu::AttachmentLoadOperation::Clear,
            .initialLayout = KDGpu::TextureLayout::Undefined, // Don't care about previous contents
        };

        // Load: Preserve previous contents
        // Use for incremental rendering or UI overlays
        // attachment.loadOperation = KDGpu::AttachmentLoadOperation::Load;
        // attachment.initialLayout = KDGpu::TextureLayout::ColorAttachmentOptimal; // Must match actual layout

        // DontCare: Undefined behavior but may be faster
        // Use when you know you'll overwrite all pixels
        // attachment.loadOperation = KDGpu::AttachmentLoadOperation::DontCare;
        // attachment.initialLayout = KDGpu::TextureLayout::Undefined;

        loadOptions.colorAttachments.push_back(attachment);
        //! [renderpass_load_operations]
    }
}

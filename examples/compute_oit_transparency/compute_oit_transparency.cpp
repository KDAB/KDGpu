/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "compute_oit_transparency.h"

#include <KDGpuExample/engine.h>
#include <KDGpuExample/kdgpuexample.h>

#include <KDGpu/bind_group_layout_options.h>
#include <KDGpu/bind_group_options.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/graphics_pipeline_options.h>
#include <KDGpu/compute_pipeline_options.h>
#include <KDGpu/texture_options.h>
#include <KDGpu/texture_view_options.h>
#include <KDGpu/gpu_core.h>

#include <glm/glm.hpp>
#include <glm/ext/matrix_projection.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <random>
#include <cassert>

using namespace KDGpu;

namespace {

constexpr size_t ParticlesCount = 1024;

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};
static_assert(sizeof(Vertex) == 6 * sizeof(float));

struct ParticleData {
    glm::vec4 positionAndRadius;
    glm::vec4 velocity;
    glm::vec4 color;
};
static_assert(sizeof(ParticleData) == 12 * sizeof(float));

std::vector<ParticleData> initializeParticlesBuffer(const size_t particlesCount)
{
    std::vector<ParticleData> particles(particlesCount);

    std::random_device rd; // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()

    std::uniform_int_distribution<> posDistrib(0, 1024);
    std::uniform_int_distribution<> colorDistrib(0, 255);
    std::uniform_int_distribution<> radiusDistrib(0, 255);

    for (ParticleData &particle : particles) {
        for (size_t i = 0; i < 3; ++i) {
            particle.positionAndRadius[i] = (float(posDistrib(gen)) - 512.0f) / 512.0f * 32.0f;
            particle.velocity[i] = float(posDistrib(gen)) / 512.0f * 0.01f;
            particle.color[i] = float(colorDistrib(gen)) / 255.0f;
        }
        particle.positionAndRadius[3] = std::fabs(float(radiusDistrib(gen)) / 255.0f) * 3.0f;
        particle.velocity[3] = 0.0f;
        particle.color[3] = 0.2f;
    }

    return particles;
}

std::vector<Vertex> initializeCubeMesh()
{
    // clang-format off
    const glm::vec3 A(1.0f, 1.0f, 1.0f);                      //       D ---------- C
    const glm::vec3 B(-1.0f, 1.0f, 1.0f);                     //      /|           /|
    const glm::vec3 C(1.0f, 1.0f, -1.0f);                     //     B ---------- A |
    const glm::vec3 D(-1.0f, 1.0f, -1.0f);                    //     | |          | |
    const glm::vec3 E(1.0f, -1.0f, 1.0f);                     //     | H ---------| G
    const glm::vec3 F(-1.0f, -1.0f, 1.0f);                    //     |/           |/
    const glm::vec3 G(1.0f, -1.0f, -1.0f);                    //     F ---------- E
    const glm::vec3 H(-1.0f, -1.0f, -1.0f);                   //

    const glm::vec3 nTop(0.0f, 1.0f, 0.0f);
    const glm::vec3 nBottom(0.0f, -1.0f, 0.0f);
    const glm::vec3 nFront(0.0f, 0.0f, -1.0f);
    const glm::vec3 nBack(0.0f, 0.0f, 1.0f);
    const glm::vec3 nLeft(1.0f, 0.0f, 0.0f);
    const glm::vec3 nRight(-1.0f, 0.0f, 0.0f);

    return {
        // Top
        {A, nTop}, {C, nTop}, {D, nTop},
        {D, nTop}, {B, nTop}, {A, nTop},
        // Front
        {B, nFront}, {F, nFront}, {E, nFront},
        {E, nFront}, {A, nFront}, {B, nFront},
        // Back
        {G, nBack}, {H, nBack}, {D, nBack},
        {D, nBack}, {C, nBack}, {G, nBack},
        // Bottom
        {E, nBottom}, {F, nBottom}, {H, nBottom},
        {H, nBottom}, {G, nBottom}, {E, nBottom},
        // Left
        {F, nLeft}, {B, nLeft}, {D, nLeft},
        {D, nLeft}, {H, nLeft}, {F, nLeft},
        // Right
        {A, nRight}, {E, nRight}, {G, nRight},
        {G, nRight}, {C, nRight}, {A, nRight},
    };
    // clang-format on
}

std::vector<Vertex> initializeSphereMesh()
{
    std::vector<Vertex> vertices;
    const uint32_t rings = 8;
    const uint32_t slices = 8;
    vertices.reserve(rings * slices * 6);

    const float dTheta = float(M_PI * 2) / static_cast<float>(slices);
    const float dPhi = float(M_PI) / static_cast<float>(rings);

    for (uint16_t r = 1; r < rings + 1; ++r) {
        const float phiN = float(M_PI_2) - static_cast<float>(r) * dPhi;
        const float phiN1 = float(M_PI_2) - static_cast<float>(r - 1) * dPhi;
        const float cosPhiN = std::cos(phiN);
        const float sinPhiN = std::sin(phiN);
        const float cosPhiN1 = std::cos(phiN1);
        const float sinPhiN1 = std::sin(phiN1);

        // Iterate over longitudes (slices)
        for (uint16_t s = 1; s < slices + 1; ++s) {
            const float theta = static_cast<float>(s) * dTheta;
            const float thetaPrev = static_cast<float>(s - 1) * dTheta;

            const float costT = std::cos(theta);
            const float sinT = std::sin(theta);
            const float costTPrev = std::cos(thetaPrev);
            const float sinTPrev = std::sin(thetaPrev);

            const glm::vec3 pNR{ costT * cosPhiN, sinPhiN, sinT * cosPhiN };
            const glm::vec3 pN1R{ costTPrev * cosPhiN, sinPhiN, sinTPrev * cosPhiN };
            const glm::vec3 pNR1{ costT * cosPhiN1, sinPhiN1, sinT * cosPhiN1 };
            const glm::vec3 pN1R1{ costTPrev * cosPhiN1, sinPhiN1, sinTPrev * cosPhiN1 };

            vertices.emplace_back(Vertex{ pNR, pNR });
            vertices.emplace_back(Vertex{ pNR1, pNR1 });
            vertices.emplace_back(Vertex{ pN1R1, pN1R1 });
            vertices.emplace_back(Vertex{ pN1R1, pN1R1 });
            vertices.emplace_back(Vertex{ pN1R, pN1R });
            vertices.emplace_back(Vertex{ pNR, pNR });
        }
    }
    assert(vertices.size() == rings * slices * 6);

    return vertices;
}

std::vector<uint8_t> updateCameraData(float width, float height)
{
    std::vector<uint8_t> rawCameraData(2 * sizeof(glm::mat4));
    const glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0.0f, -15.0f, 100.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 projectionMatrix = glm::perspective(45.0f, width / height, 0.1f, 1000.0f);

    std::memcpy(rawCameraData.data(), glm::value_ptr(viewMatrix), sizeof(glm::mat4));
    std::memcpy(rawCameraData.data() + sizeof(glm::mat4), glm::value_ptr(projectionMatrix), sizeof(glm::mat4));

    return rawCameraData;
}

} // namespace

void ComputeOitTransparency::initializeGlobal()
{
    auto initializeBuffers = [this]() -> void {
        // Create a buffer to hold particles data (will be used as per Instance data)

        const std::vector<uint8_t> rawCameraData = updateCameraData(1.0f, 1.0f);
        m_global.cameraDataBuffer = m_device.createBuffer(BufferOptions{
                                                                  .size = rawCameraData.size(),
                                                                  .usage = BufferUsageFlagBits::UniformBufferBit,
                                                                  .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
                                                          },
                                                          rawCameraData.data());
    };
    initializeBuffers();

    m_global.cameraBindGroupLayout = m_device.createBindGroupLayout(BindGroupLayoutOptions{
            .bindings = {
                    {
                            .binding = 0,
                            .resourceType = ResourceBindingType::UniformBuffer,
                            .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit),
                    },
            },
    });

    m_global.cameraBindGroup = m_device.createBindGroup(BindGroupOptions{
            .layout = m_global.cameraBindGroupLayout,
            .resources = {
                    {
                            .binding = 0,
                            .resource = UniformBufferBinding{ .buffer = m_global.cameraDataBuffer },
                    },
            },
    });
}

void ComputeOitTransparency::initializeParticles()
{
    auto initializeBuffers = [this]() -> void {
        // Create a buffer to hold particles data (will be used as per Instance data)
        const std::vector<ParticleData> particles = initializeParticlesBuffer(ParticlesCount);
        m_particles.particleDataBuffer = m_device.createBuffer(BufferOptions{
                                                                       .size = ParticlesCount * sizeof(ParticleData),
                                                                       .usage = BufferUsageFlagBits::VertexBufferBit | BufferUsageFlagBits::StorageBufferBit,
                                                                       .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
                                                               },
                                                               particles.data());
    };
    initializeBuffers();

    auto initializeComputePipeline = [this]() -> void {
        // Create a compute shader (spir-v only for now)
        auto computeShaderPath = KDGpuExample::assetDir().file("shaders/examples/compute_oit_transparency/particles.comp.spv");
        auto computeShader = m_device.createShaderModule(KDGpuExample::readShaderFile(computeShaderPath));

        // Create bind group layout consisting of a single binding holding a SSBO
        const BindGroupLayout bindGroupLayout = m_device.createBindGroupLayout(BindGroupLayoutOptions{
                .bindings = {
                        {
                                .binding = 0,
                                .resourceType = ResourceBindingType::StorageBuffer,
                                .shaderStages = ShaderStageFlags(ShaderStageFlagBits::ComputeBit),
                        },
                },
        });

        // Create a pipeline layout (array of bind group layouts)
        m_particles.computePipelineLayout = m_device.createPipelineLayout(PipelineLayoutOptions{
                .bindGroupLayouts = { bindGroupLayout } });

        // Create a bindGroup to hold the UBO with the transform
        m_particles.particleBindGroup = m_device.createBindGroup(BindGroupOptions{
                .layout = bindGroupLayout,
                .resources = {
                        {
                                .binding = 0,
                                .resource = StorageBufferBinding{ .buffer = m_particles.particleDataBuffer },
                        },
                },
        });

        m_particles.computePipeline = m_device.createComputePipeline(ComputePipelineOptions{
                .layout = m_particles.computePipelineLayout,
                .shaderStage = {
                        .shaderModule = computeShader,
                        // Use a specialization constant to set the local X workgroup size
                        .specializationConstants = {
                                {
                                        .constantId = 0,
                                        .value = 256,
                                },
                        },
                },
        });
    };
    initializeComputePipeline();
}

void ComputeOitTransparency::initializeAlpha()
{
    m_alpha.renderPassOptions = {
        .colorAttachments = {},
        .depthStencilAttachment = {}
    };

    m_alpha.alphaBindGroupLayout = m_device.createBindGroupLayout(BindGroupLayoutOptions{
            .bindings = {
                    {
                            .binding = 0,
                            .resourceType = ResourceBindingType::StorageBuffer,
                            .shaderStages = ShaderStageFlags(KDGpu::ShaderStageFlagBits::FragmentBit),
                    },
                    {
                            .binding = 1,
                            .resourceType = ResourceBindingType::StorageImage,
                            .shaderStages = ShaderStageFlags(KDGpu::ShaderStageFlagBits::FragmentBit),
                    },
            },
    });

    m_alpha.alphaLinkedListBindGroup = m_device.createBindGroup(BindGroupOptions{
            .layout = m_alpha.alphaBindGroupLayout,
            .resources = {},
    });

    // fragmentHeadsPointer Texture and TextureView and the fragmentLinkedListBuffer SSBO are
    // created when resize() is called given their sizes depend on the window extent.
    // The alphaLinkedListBindGroup is updated at that point as well.
}

void ComputeOitTransparency::initializeCompositing()
{
    auto initializeGraphicsPipeline = [this]() -> void {
        // Create a vertex shader and fragment shader (spir-v only for now)
        auto vertexShaderPath = KDGpuExample::assetDir().file("shaders/examples/compute_oit_transparency/compositing.vert.spv");
        auto vertexShader = m_device.createShaderModule(KDGpuExample::readShaderFile(vertexShaderPath));

        auto fragmentShaderPath = KDGpuExample::assetDir().file("shaders/examples/compute_oit_transparency/compositing.frag.spv");
        auto fragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(fragmentShaderPath));

        // Create a pipeline layout (array of bind group layouts)
        m_compositing.graphicsPipelineLayout = m_device.createPipelineLayout(PipelineLayoutOptions{
                .bindGroupLayouts = { m_alpha.alphaBindGroupLayout },
        });

        // Create a pipeline
        m_compositing.graphicsPipeline = m_device.createGraphicsPipeline(GraphicsPipelineOptions{
                .shaderStages = {
                        { .shaderModule = vertexShader, .stage = ShaderStageFlagBits::VertexBit },
                        { .shaderModule = fragmentShader, .stage = ShaderStageFlagBits::FragmentBit } },
                .layout = m_compositing.graphicsPipelineLayout,
                .vertex = {
                        .buffers = {},
                        .attributes = {},
                },
                .renderTargets = {
                        { .format = m_swapchainFormat },
                },
                .depthStencil = {
                        .format = m_depthFormat,
                        .depthWritesEnabled = false,
                        .depthCompareOperation = CompareOperation::Less,
                },
        });

        m_compositing.renderPassOptions = {
            .colorAttachments = {
                    {
                            .view = {}, // Not setting the swapchain texture view just yet
                            .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f },
                            .initialLayout = TextureLayout::Undefined,
                            .finalLayout = TextureLayout::PresentSrc,
                    },
            },
            .depthStencilAttachment = {
                    .view = m_depthTextureView,
            },
        };
    };
    initializeGraphicsPipeline();
}

void ComputeOitTransparency::initializeMeshes()
{
    auto initializeSphereBuffers = [this]() -> void {
        const std::vector<Vertex> vertices = initializeSphereMesh();
        m_sphereMesh.vertexBuffer = m_device.createBuffer(BufferOptions{
                                                                  .size = vertices.size() * sizeof(Vertex),
                                                                  .usage = BufferUsageFlagBits::VertexBufferBit | BufferUsageFlagBits::StorageBufferBit,
                                                                  .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
                                                          },
                                                          vertices.data());
        m_sphereMesh.vertexCount = vertices.size();
    };
    initializeSphereBuffers();

    auto initializeCubeBuffers = [this]() -> void {
        const std::vector<Vertex> vertices = initializeCubeMesh();
        m_cubeMesh.vertexBuffer = m_device.createBuffer(BufferOptions{
                                                                .size = vertices.size() * sizeof(Vertex),
                                                                .usage = BufferUsageFlagBits::VertexBufferBit | BufferUsageFlagBits::StorageBufferBit,
                                                                .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
                                                        },
                                                        vertices.data());
    };
    initializeCubeBuffers();

    auto initializeSphereMeshPipeline = [this]() -> void {
        // Create a vertex shader and fragment shader (spir-v only for now)
        auto vertexShaderPath = KDGpuExample::assetDir().file("shaders/examples/compute_oit_transparency/sphere_instanced.vert.spv");
        auto vertexShader = m_device.createShaderModule(KDGpuExample::readShaderFile(vertexShaderPath));

        auto fragmentShaderPath = KDGpuExample::assetDir().file("shaders/examples/compute_oit_transparency/alpha.frag.spv");
        auto fragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(fragmentShaderPath));

        // Create a pipeline layout (array of bind group layouts)
        m_sphereMesh.graphicsPipelineLayout = m_device.createPipelineLayout(PipelineLayoutOptions{
                .bindGroupLayouts = { m_alpha.alphaBindGroupLayout, m_global.cameraBindGroupLayout },
        });

        // Create a pipeline
        m_sphereMesh.graphicsPipeline = m_device.createGraphicsPipeline(GraphicsPipelineOptions{
                .shaderStages = {
                        { .shaderModule = vertexShader, .stage = ShaderStageFlagBits::VertexBit },
                        { .shaderModule = fragmentShader, .stage = ShaderStageFlagBits::FragmentBit } },
                .layout = m_sphereMesh.graphicsPipelineLayout,
                .vertex = {
                        .buffers = {
                                { .binding = 0, .stride = sizeof(Vertex) },
                                { .binding = 1, .stride = sizeof(ParticleData), .inputRate = VertexRate::Instance },
                        },
                        .attributes = {
                                { .location = 0, .binding = 0, .format = Format::R32G32B32_SFLOAT }, // Vertex Position
                                { .location = 1, .binding = 0, .format = Format::R32G32B32_SFLOAT, .offset = sizeof(glm::vec3) }, // Vertex Normal
                                { .location = 2, .binding = 1, .format = Format::R32G32B32A32_SFLOAT }, // Particle Position
                                { .location = 3, .binding = 1, .format = Format::R32G32B32A32_SFLOAT, .offset = 2 * sizeof(glm::vec4) }, // Particle Color
                        },
                },
                .renderTargets = {},
                .depthStencil = {
                        .format = KDGpu::Format::UNDEFINED,
                        .depthTestEnabled = false,
                        .depthWritesEnabled = false,
                        .depthCompareOperation = CompareOperation::Always,
                },
                .primitive = {
                        .cullMode = CullModeFlagBits::BackBit,
                },
        });
    };
    initializeSphereMeshPipeline();

    auto initializeCubeMeshPipeline = [this]() -> void {
        // Create a vertex shader and fragment shader (spir-v only for now)
        auto vertexShaderPath = KDGpuExample::assetDir().file("shaders/examples/compute_oit_transparency/cube.vert.spv");
        auto vertexShader = m_device.createShaderModule(KDGpuExample::readShaderFile(vertexShaderPath));

        auto fragmentShaderPath = KDGpuExample::assetDir().file("shaders/examples/compute_oit_transparency/alpha.frag.spv");
        auto fragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(fragmentShaderPath));

        // Create a pipeline layout (array of bind group layouts)
        m_cubeMesh.graphicsPipelineLayout = m_device.createPipelineLayout(PipelineLayoutOptions{
                .bindGroupLayouts = { m_alpha.alphaBindGroupLayout, m_global.cameraBindGroupLayout },
        });

        // Create a pipeline
        m_cubeMesh.graphicsPipeline = m_device.createGraphicsPipeline(GraphicsPipelineOptions{
                .shaderStages = {
                        { .shaderModule = vertexShader, .stage = ShaderStageFlagBits::VertexBit },
                        { .shaderModule = fragmentShader, .stage = ShaderStageFlagBits::FragmentBit } },
                .layout = m_cubeMesh.graphicsPipelineLayout,
                .vertex = {
                        .buffers = {
                                { .binding = 0, .stride = sizeof(Vertex) },
                        },
                        .attributes = {
                                { .location = 0, .binding = 0, .format = Format::R32G32B32_SFLOAT }, // Vertex Position
                                { .location = 1, .binding = 0, .format = Format::R32G32B32_SFLOAT, .offset = sizeof(glm::vec3) }, // Vertex Normal
                        },
                },
                .renderTargets = {},
                .depthStencil = {
                        .format = KDGpu::Format::UNDEFINED,
                        .depthTestEnabled = false,
                        .depthWritesEnabled = false,
                        .depthCompareOperation = CompareOperation::Always,
                },
                .primitive = {
                        .cullMode = CullModeFlagBits::None,
                },
        });
    };
    initializeCubeMeshPipeline();
}

void ComputeOitTransparency::initializeScene()
{
    initializeGlobal();
    initializeParticles();
    initializeAlpha();
    initializeCompositing();
    initializeMeshes();

    resize();
}

void ComputeOitTransparency::cleanupScene()
{
    m_particles = {};
    m_alpha = {};
    m_compositing = {};
    m_cubeMesh = {};
    m_sphereMesh = {};
    m_global = {};
}

void ComputeOitTransparency::updateScene()
{
}

void ComputeOitTransparency::resize()
{
    // Swapchain might have been resized and texture views recreated. Ensure we update the PassOptions accordingly
    m_compositing.renderPassOptions.depthStencilAttachment.view = m_depthTextureView;

    // Set FrameBuffer size to use for the Alpha pass which has no color attachment (hence we can't retrieve the framebuffer dimensions from the attachment)
    m_alpha.renderPassOptions.framebufferWidth = m_window->width();
    m_alpha.renderPassOptions.framebufferHeight = m_window->height();
    m_alpha.renderPassOptions.framebufferArrayLayers = 1;

    // Recreated fragmentHeadsPointer texture
    m_alpha.fragmentHeadsPointer = m_device.createTexture(KDGpu::TextureOptions{
            .label = "fragmentHeadPointers",
            .type = KDGpu::TextureType::TextureType2D,
            .format = KDGpu::Format::R32_UINT,
            .extent = { std::max(m_window->width(), uint32_t(1)), std::max(m_window->height(), uint32_t(1)), 1 },
            .mipLevels = 1,
            .usage =
                    KDGpu::TextureUsageFlagBits::TransferDstBit | KDGpu::TextureUsageFlagBits::StorageBit,
            .memoryUsage = KDGpu::MemoryUsage::GpuOnly,
    });
    m_alpha.fragmentHeadsPointerView = m_alpha.fragmentHeadsPointer.createView(KDGpu::TextureViewOptions{
            .label = "fragmentHeadPointersView",
            .range = {
                    .aspectMask = KDGpu::TextureAspectFlagBits::ColorBit,
                    .levelCount = 1,
            },
    });
    m_alpha.fragmentHeadsPointerLayout = TextureLayout::Undefined;

    // Recreate fragmentsLinkedList SSBO
    const size_t MaxFragmentCount = std::max(m_window->width(), uint32_t(1)) * std::max(m_window->height(), uint32_t(1)) * 8;

    struct FragmentInfo {
        glm::vec4 color;
        float depth;
        int32_t next;
        float _pad[2];
    };
    static_assert(sizeof(FragmentInfo) == 8 * sizeof(float));

    // vec4 to hold nextId + array of structs
    m_alpha.fragmentLinkedListBufferByteSize = sizeof(float) * 4 + MaxFragmentCount * sizeof(FragmentInfo);
    m_alpha.fragmentLinkedListBuffer = m_device.createBuffer(KDGpu::BufferOptions{
            .label = "FragmentSSBO",
            .size = m_alpha.fragmentLinkedListBufferByteSize,
            .usage = KDGpu::BufferUsageFlagBits::StorageBufferBit |
                    KDGpu::BufferUsageFlagBits::TransferDstBit,
            .memoryUsage = KDGpu::MemoryUsage::GpuOnly,
    });

    m_alpha.alphaLinkedListBindGroup.update(KDGpu::BindGroupEntry{
            .binding = 0,
            .resource =
                    KDGpu::StorageBufferBinding{
                            .buffer = m_alpha.fragmentLinkedListBuffer,
                    },
    });
    m_alpha.alphaLinkedListBindGroup.update(KDGpu::BindGroupEntry{
            .binding = 1,
            .resource =
                    KDGpu::ImageBinding{
                            .textureView = m_alpha.fragmentHeadsPointerView,
                    },
    });

    void *cameraData = m_global.cameraDataBuffer.map();
    const std::vector<uint8_t> rawCameraData = updateCameraData(m_window->width(), std::max(m_window->height(), uint32_t(1)));
    std::memcpy(cameraData, rawCameraData.data(), 2 * sizeof(glm::mat4));
    m_global.cameraDataBuffer.unmap();
}

void ComputeOitTransparency::render()
{
    auto commandRecorder = m_device.createCommandRecorder();
    {
        // Compute
        {
            auto computePass = commandRecorder.beginComputePass();
            computePass.setPipeline(m_particles.computePipeline);
            computePass.setBindGroup(0, m_particles.particleBindGroup);
            constexpr size_t LocalWorkGroupXSize = 256;
            computePass.dispatchCompute(ComputeCommand{ .workGroupX = ParticlesCount / LocalWorkGroupXSize });
            computePass.end();
        }

        // Alpha
        {
            // Wait for SSBO writes completion by ComputeShader
            commandRecorder.bufferMemoryBarrier(KDGpu::BufferMemoryBarrierOptions{
                    .srcStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::ComputeShaderBit),
                    .srcMask = KDGpu::AccessFlagBit::ShaderWriteBit,
                    .dstStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::VertexInputBit),
                    .dstMask = KDGpu::AccessFlagBit::VertexAttributeReadBit,
                    .buffer = m_particles.particleDataBuffer,
            });

            // Clear Fragment List SSBO
            commandRecorder.clearBuffer(KDGpu::BufferClear{
                    .dstBuffer = m_alpha.fragmentLinkedListBuffer,
                    .byteSize = m_alpha.fragmentLinkedListBufferByteSize,
            });

            // Transition fragmentHeadsPointer to general layout if needed
            if (m_alpha.fragmentHeadsPointerLayout == KDGpu::TextureLayout::Undefined) {
                commandRecorder.textureMemoryBarrier(KDGpu::TextureMemoryBarrierOptions{
                        .srcStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::TopOfPipeBit),
                        .srcMask = KDGpu::AccessFlagBit::None,
                        .dstStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::TransferBit),
                        .dstMask =
                                KDGpu::AccessFlagBit::TransferWriteBit | KDGpu::AccessFlagBit::TransferReadBit,
                        .oldLayout = KDGpu::TextureLayout::Undefined,
                        .newLayout = KDGpu::TextureLayout::General,
                        .texture = m_alpha.fragmentHeadsPointer,
                        .range = {
                                .aspectMask = KDGpu::TextureAspectFlagBits::ColorBit,
                                .levelCount = 1,
                        },
                });
                m_alpha.fragmentHeadsPointerLayout = KDGpu::TextureLayout::General;
            }

            // Clear Fragment Head Texture Image
            commandRecorder.clearColorTexture(KDGpu::ClearColorTexture{
                    .texture = m_alpha.fragmentHeadsPointer,
                    .layout = KDGpu::TextureLayout::General,
                    .clearValue = {
                            .uint32 = { 0, 0, 0, 0 },
                    },
                    .ranges = {
                            {
                                    .aspectMask = KDGpu::TextureAspectFlagBits::ColorBit,
                                    .levelCount = 1,
                            },
                    },
            });

            // Wait until fragments SSBO has been cleared
            commandRecorder.bufferMemoryBarrier(KDGpu::BufferMemoryBarrierOptions{
                    .srcStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::TransferBit),
                    .srcMask = KDGpu::AccessFlagBit::TransferWriteBit,
                    .dstStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::FragmentShaderBit),
                    .dstMask = KDGpu::AccessFlagBit::ShaderWriteBit | KDGpu::AccessFlagBit::ShaderReadBit,
                    .buffer = m_alpha.fragmentLinkedListBuffer,
            });

            // Wait until fragments SSBO Heads pointer image has been cleared
            commandRecorder.textureMemoryBarrier(KDGpu::TextureMemoryBarrierOptions{
                    .srcStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::TransferBit),
                    .srcMask = KDGpu::AccessFlagBit::TransferWriteBit,
                    .dstStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::FragmentShaderBit),
                    .dstMask = KDGpu::AccessFlagBit::ShaderWriteBit | KDGpu::AccessFlagBit::ShaderReadBit,
                    .oldLayout = KDGpu::TextureLayout::General,
                    .newLayout = KDGpu::TextureLayout::General,
                    .texture = m_alpha.fragmentHeadsPointer,
                    .range = {
                            .aspectMask = KDGpu::TextureAspectFlagBits::ColorBit,
                            .levelCount = 1,
                    },
            });

            // Render Alpha meshes to fragment list
            auto alphaPass = commandRecorder.beginRenderPass(m_alpha.renderPassOptions);

            // Draw Spheres
            alphaPass.setPipeline(m_sphereMesh.graphicsPipeline);
            alphaPass.setBindGroup(0, m_alpha.alphaLinkedListBindGroup);
            alphaPass.setBindGroup(1, m_global.cameraBindGroup);
            alphaPass.setVertexBuffer(0, m_sphereMesh.vertexBuffer);
            alphaPass.setVertexBuffer(1, m_particles.particleDataBuffer); // Per instance Data
            alphaPass.draw(DrawCommand{ .vertexCount = uint32_t(m_sphereMesh.vertexCount), .instanceCount = ParticlesCount });

            // Draw Cube
            alphaPass.setPipeline(m_cubeMesh.graphicsPipeline);
            alphaPass.setBindGroup(0, m_alpha.alphaLinkedListBindGroup);
            alphaPass.setBindGroup(1, m_global.cameraBindGroup);
            alphaPass.setVertexBuffer(0, m_cubeMesh.vertexBuffer);
            alphaPass.draw(DrawCommand{ .vertexCount = 36, .instanceCount = 1 });

            alphaPass.end();
        }

        // Compositing
        {
            // Wait until fragment Heads pointer image writes have been completed
            commandRecorder.textureMemoryBarrier(KDGpu::TextureMemoryBarrierOptions{
                    .srcStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::FragmentShaderBit),
                    .srcMask = KDGpu::AccessFlagBit::ShaderWriteBit,
                    .dstStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::FragmentShaderBit),
                    .dstMask = KDGpu::AccessFlagBit::ShaderReadBit,
                    .oldLayout = KDGpu::TextureLayout::General,
                    .newLayout = KDGpu::TextureLayout::General,
                    .texture = m_alpha.fragmentHeadsPointer,
                    .range = {
                            .aspectMask = KDGpu::TextureAspectFlagBits::ColorBit,
                            .levelCount = 1,
                    },
            });
            // Wait until fragment SSBO list writes have been completed
            commandRecorder.bufferMemoryBarrier(KDGpu::BufferMemoryBarrierOptions{
                    .srcStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::FragmentShaderBit),
                    .srcMask = KDGpu::AccessFlagBit::ShaderWriteBit,
                    .dstStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::FragmentShaderBit),
                    .dstMask = KDGpu::AccessFlagBit::ShaderReadBit,
                    .buffer = m_alpha.fragmentLinkedListBuffer,
            });

            // Render Compositing full screen quad to screen
            m_compositing.renderPassOptions.colorAttachments[0].view = m_swapchainViews.at(m_currentSwapchainImageIndex);
            auto compositingPass = commandRecorder.beginRenderPass(m_compositing.renderPassOptions);
            compositingPass.setPipeline(m_compositing.graphicsPipeline);
            compositingPass.setBindGroup(0, m_alpha.alphaLinkedListBindGroup);
            compositingPass.draw(DrawCommand{ .vertexCount = 6 });
            compositingPass.end();
        }
    }
    m_global.commandBuffer = commandRecorder.finish();

    // Submit Commands
    const SubmitOptions submitOptions = {
        .commandBuffers = { m_global.commandBuffer },
        .waitSemaphores = { m_presentCompleteSemaphores[m_inFlightIndex] },
        .signalSemaphores = { m_renderCompleteSemaphores[m_currentSwapchainImageIndex] }
    };
    m_queue.submit(submitOptions);
}

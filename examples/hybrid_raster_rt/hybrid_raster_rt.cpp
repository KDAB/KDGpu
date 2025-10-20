/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "hybrid_raster_rt.h"

#include <KDGpuExample/engine.h>
#include <KDGpuExample/kdgpuexample.h>
#include <KDGpuExample/view_projection.h>

#include <KDGpu/bind_group_layout_options.h>
#include <KDGpu/bind_group_options.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/graphics_pipeline_options.h>
#include <KDGpu/compute_pipeline_options.h>
#include <KDGpu/texture_options.h>
#include <KDGpu/texture_view_options.h>
#include <KDGpu/gpu_core.h>
#include <KDGpu/raytracing_pipeline_options.h>
#include <KDGpu/raytracing_pass_command_recorder.h>

#include <glm/glm.hpp>
#include <glm/ext/matrix_projection.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <random>
#include <cassert>

using namespace KDGpu;
using namespace KDGpuExample;

/**
 * This example demonstrate an advance use of raytracing and raster graphics used together.

   The scene is composed of:
   - An Opaque Ground Plane
   - Multiple Alpha Blended Spheres
   - Multiple Opaque Spheres

   1) The sphere's positions are animated using a compute shader
   2) A Deferred Rendering approach is used where we record Depth in a Z-Fill pre-pass and then record
      WorldPosition and WorldNormals and Colors for the Opaque Meshes.
   3) For Alpha Blended meshes, we use a linked list to store alpha fragments along with their depth
   4) For all meshes, we generate AccelerationStructures. This allows use to use RayTracing to
      create a Shadow Texture. Essentially for each world position we recorded in our GBuffer, we compute
      a ray from that world position to the light source. Any Intersection against that ray means that
      some other mesh is obstructing the light, hence we have shadows.
   5) Finally we composite everything together by:
      - Retrieving the color for Opaque Meshes
      - Sorting Alpha fragments by depth and blending against the opaque color
      - Retrieve Shadow information and modifying the color accordingly
 */

namespace {

constexpr size_t AlphaSpheresCount = 768;
constexpr size_t OpaqueSpheresCount = 256;
constexpr size_t ParticlesCount = AlphaSpheresCount + OpaqueSpheresCount;

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

std::vector<ParticleData> initializeParticlesBuffer(const size_t particlesCount, float alpha)
{
    std::vector<ParticleData> particles(particlesCount);

    std::random_device rd; // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()

    std::uniform_int_distribution<> posDistrib(0, 1024);
    std::uniform_int_distribution<> colorDistrib(0, 255);
    std::uniform_int_distribution<> radiusDistrib(0, 255);

    for (ParticleData &particle : particles) {
        for (size_t i = 0; i < 3; ++i) {
            particle.positionAndRadius[i] = (float(posDistrib(gen)) - 512.0f) / 512.0f * 50.0f;
            particle.velocity[i] = float(posDistrib(gen)) / 512.0f * 0.01f;
            particle.color[i] = float(colorDistrib(gen)) / 255.0f;
        }
        particle.positionAndRadius[3] = std::fabs(float(radiusDistrib(gen)) / 255.0f) * 2.0f;
        particle.velocity[3] = 0.0f;
        particle.color[3] = alpha;
    }

    return particles;
}

std::vector<Vertex> initializePlaneMesh()
{
    // clang-format off
    const float scale = 52.0f;
    const glm::vec3 A = glm::vec3(2.0f, 1.0f, 2.0f) * scale;  //       D ---------- C
    const glm::vec3 B = glm::vec3(-2.0f, 1.0f, 2.0f) * scale; //      /            /
    const glm::vec3 C = glm::vec3(2.0f, 1.0f, -2.0f) * scale; //     B ---------- A
    const glm::vec3 D = glm::vec3(-2.0f, 1.0f, -2.0f) * scale;

    const glm::vec3 n(0.0f, 1.0f, 0.0f);

    return {
        // Top
        {A, n}, {C, n}, {D, n},
        {D, n}, {B, n}, {A, n},
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
    const glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0.0f, -50.0f, -150.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 projectionMatrix = glm::perspective(45.0f, width / height, 0.1f, 1000.0f);

    std::memcpy(rawCameraData.data(), glm::value_ptr(viewMatrix), sizeof(glm::mat4));
    std::memcpy(rawCameraData.data() + sizeof(glm::mat4), glm::value_ptr(projectionMatrix), sizeof(glm::mat4));

    return rawCameraData;
}

} // namespace

void HybridRasterRt::initializeGlobal()
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

    m_global.lightPosPushConstant = PushConstantRange{
        .offset = 0,
        .size = sizeof(glm::vec3),
        .shaderStages = ShaderStageFlagBits::VertexBit | ShaderStageFlagBits::FragmentBit | ShaderStageFlagBits::RaygenBit,
    };
    m_global.lightPos = glm::vec3(-10.0f, 100.0f, 10.0f);
}

void HybridRasterRt::initializeParticles()
{
    auto initializeBuffers = [this]() -> void {
        // Create a buffer to hold particles data (will be used as per Instance data)
        std::vector<ParticleData> particles = initializeParticlesBuffer(OpaqueSpheresCount, 1.0f);
        const std::vector<ParticleData> alphaParticles = initializeParticlesBuffer(AlphaSpheresCount, 0.25f);
        particles.insert(particles.end(), alphaParticles.begin(), alphaParticles.end());
        assert(particles.size() == ParticlesCount);

        m_particles.particleDataBuffer = m_device.createBuffer(BufferOptions{
                                                                       .size = ParticlesCount * sizeof(ParticleData),
                                                                       .usage = BufferUsageFlagBits::VertexBufferBit | BufferUsageFlagBits::StorageBufferBit,
                                                                       .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
                                                               },
                                                               particles.data());

        m_particles.blasTransformBuffer = m_device.createBuffer(BufferOptions{
                .size = ParticlesCount * sizeof(VkTransformMatrixKHR),
                .usage = BufferUsageFlagBits::ShaderDeviceAddressBit | BufferUsageFlagBits::AccelerationStructureBuildInputReadOnlyBit | BufferUsageFlagBits::StorageBufferBit,
                .memoryUsage = MemoryUsage::GpuOnly,
        });
    };
    initializeBuffers();

    auto initializeComputePipeline = [this]() -> void {
        // Create a compute shader (spir-v only for now)
        auto computeShaderPath = KDGpuExample::assetDir().file("shaders/examples/hybrid_raster_rt/particles.comp.spv");
        auto computeShader = m_device.createShaderModule(KDGpuExample::readShaderFile(computeShaderPath));

        // Create bind group layout consisting of a single binding holding a SSBO
        const BindGroupLayout bindGroupLayout = m_device.createBindGroupLayout(BindGroupLayoutOptions{
                .bindings = {
                        {
                                .binding = 0,
                                .resourceType = ResourceBindingType::StorageBuffer,
                                .shaderStages = ShaderStageFlags(ShaderStageFlagBits::ComputeBit),
                        },
                        {
                                .binding = 1,
                                .resourceType = ResourceBindingType::StorageBuffer,
                                .shaderStages = ShaderStageFlags(ShaderStageFlagBits::ComputeBit),
                        },
                },
        });

        // Create a pipeline layout (array of bind group layouts)
        m_particles.computePipelineLayout = m_device.createPipelineLayout(PipelineLayoutOptions{
                .bindGroupLayouts = { bindGroupLayout },
        });

        // Create a bindGroup to hold the UBO with the transform
        m_particles.particleBindGroup = m_device.createBindGroup(BindGroupOptions{
                .layout = bindGroupLayout,
                .resources = {
                        {
                                .binding = 0,
                                .resource = StorageBufferBinding{ .buffer = m_particles.particleDataBuffer },
                        },
                        {
                                .binding = 1,
                                .resource = StorageBufferBinding{ .buffer = m_particles.blasTransformBuffer },
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

void HybridRasterRt::initializeGBuffer()
{
    m_gbuffer.initialize(&m_device);
}

void HybridRasterRt::initializeAlpha()
{
    m_alphaPass.renderPassOptions = {
        .colorAttachments = {},
        .depthStencilAttachment = {}
    };
}

void HybridRasterRt::initializeShadows()
{
    // Create raytracing shaders
    auto rayTracingGenShaderPath = KDGpuExample::assetDir().file("shaders/examples/hybrid_raster_rt/shadow.rgen.spv");
    auto rayTracingMissShaderPath = KDGpuExample::assetDir().file("shaders/examples/hybrid_raster_rt/shadow.rmiss.spv");
    auto rayTracingAnyHitShaderPath = KDGpuExample::assetDir().file("shaders/examples/hybrid_raster_rt/shadow.rahit.spv");
    auto rayTracingClosestHitShaderPath = KDGpuExample::assetDir().file("shaders/examples/hybrid_raster_rt/shadow.rchit.spv");

    auto rayTracingGenShader = m_device.createShaderModule(readShaderFile(rayTracingGenShaderPath));
    auto rayTracingMissShader = m_device.createShaderModule(readShaderFile(rayTracingMissShaderPath));
    auto rayTracingAnyHitShader = m_device.createShaderModule(readShaderFile(rayTracingAnyHitShaderPath));
    auto rayTracingClosestHitShader = m_device.createShaderModule(readShaderFile(rayTracingClosestHitShaderPath));

    // Create a pipeline layout (array of bind group layouts)
    m_shadowPass.pipelineLayout = m_device.createPipelineLayout({
            .label = "RTShadows",
            .bindGroupLayouts = { m_gbuffer.opaqueNormalDepthBindGroupLayout, m_gbuffer.shadowBindGroupLayout, m_as.tsASBindGroupLayout },
            .pushConstantRanges = { m_global.lightPosPushConstant },
    });

    // Create a raytracing pipeline
    m_shadowPass.pipeline = m_device.createRayTracingPipeline({
            .shaderStages = {
                    ShaderStage{
                            .shaderModule = rayTracingGenShader.handle(),
                            .stage = ShaderStageFlagBits::RaygenBit,
                    },
                    ShaderStage{
                            .shaderModule = rayTracingMissShader.handle(),
                            .stage = ShaderStageFlagBits::MissBit,
                    },
                    ShaderStage{
                            .shaderModule = rayTracingAnyHitShader.handle(),
                            .stage = ShaderStageFlagBits::AnyHitBit, // For Alpha BLAS
                    },
                    ShaderStage{
                            .shaderModule = rayTracingClosestHitShader.handle(),
                            .stage = ShaderStageFlagBits::ClosestHitBit, // For Opaque BLAS
                    },
            },
            .shaderGroups = {
                    // Gen
                    RayTracingShaderGroupOptions{
                            .type = RayTracingShaderGroupType::General,
                            .generalShaderIndex = 0,
                    },
                    // Miss
                    RayTracingShaderGroupOptions{
                            .type = RayTracingShaderGroupType::General,
                            .generalShaderIndex = 1,
                    },
                    // Hit
                    RayTracingShaderGroupOptions{
                            .type = RayTracingShaderGroupType::TrianglesHit,
                            .closestHitShaderIndex = 3,
                            .anyHitShaderIndex = 2,
                    },
            },
            .layout = m_shadowPass.pipelineLayout,
    });

    // Shader Binding Table
    m_shadowPass.sbt = RayTracingShaderBindingTable(&m_device, RayTracingShaderBindingTableOptions{
                                                                       .nbrMissShaders = 1,
                                                                       .nbrHitShaders = 1,
                                                               });

    m_shadowPass.sbt.addRayGenShaderGroup(m_shadowPass.pipeline, 0);
    m_shadowPass.sbt.addMissShaderGroup(m_shadowPass.pipeline, 1);
    m_shadowPass.sbt.addHitShaderGroup(m_shadowPass.pipeline, 2);
}

void HybridRasterRt::initializeCompositing()
{
    auto initializeGraphicsPipeline = [this]() -> void {
        // Create a vertex shader and fragment shader (spir-v only for now)
        auto vertexShaderPath = KDGpuExample::assetDir().file("shaders/examples/hybrid_raster_rt/compositing.vert.spv");
        auto vertexShader = m_device.createShaderModule(KDGpuExample::readShaderFile(vertexShaderPath));

        auto fragmentShaderPath = KDGpuExample::assetDir().file("shaders/examples/hybrid_raster_rt/compositing.frag.spv");
        auto fragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(fragmentShaderPath));

        // Create a pipeline layout (array of bind group layouts)
        m_compositing.graphicsPipelineLayout = m_device.createPipelineLayout(PipelineLayoutOptions{
                .bindGroupLayouts = {
                        m_gbuffer.opaqueNormalDepthBindGroupLayout,
                        m_gbuffer.alphaBindGroupLayout,
                        m_gbuffer.shadowBindGroupLayout,
                },
        });

        // Create a pipeline
        m_compositing.graphicsPipeline = m_device.createGraphicsPipeline(GraphicsPipelineOptions{
                .label = "Compositing",
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
                            .finalLayout = TextureLayout::ColorAttachmentOptimal,
                    },
            },
            .depthStencilAttachment = {
                    .view = m_depthTextureView,
            },
        };
    };
    initializeGraphicsPipeline();
}

void HybridRasterRt::initializeLightDisplay()
{
    auto initializeGraphicsPipeline = [this]() -> void {
        // Create a vertex shader and fragment shader (spir-v only for now)
        auto vertexShaderPath = KDGpuExample::assetDir().file("shaders/examples/hybrid_raster_rt/light.vert.spv");
        auto vertexShader = m_device.createShaderModule(KDGpuExample::readShaderFile(vertexShaderPath));

        auto fragmentShaderPath = KDGpuExample::assetDir().file("shaders/examples/hybrid_raster_rt/light.frag.spv");
        auto fragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(fragmentShaderPath));

        // Create a pipeline layout (array of bind group layouts)
        m_lightDisplayPass.graphicsPipelineLayout = m_device.createPipelineLayout(PipelineLayoutOptions{
                .bindGroupLayouts = { m_global.cameraBindGroupLayout },
                .pushConstantRanges = { m_global.lightPosPushConstant },
        });

        // Create a pipeline
        m_lightDisplayPass.graphicsPipeline = m_device.createGraphicsPipeline(GraphicsPipelineOptions{
                .label = "LightDisplay",
                .shaderStages = {
                        { .shaderModule = vertexShader, .stage = ShaderStageFlagBits::VertexBit },
                        { .shaderModule = fragmentShader, .stage = ShaderStageFlagBits::FragmentBit } },
                .layout = m_lightDisplayPass.graphicsPipelineLayout,
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
                        .depthCompareOperation = CompareOperation::Always,
                },
                .primitive = {
                        .topology = PrimitiveTopology::LineList,
                },
        });

        m_lightDisplayPass.renderPassOptions = {
            .colorAttachments = {
                    {
                            .view = {}, // Not setting the swapchain texture view just yet
                            .loadOperation = AttachmentLoadOperation::Load,
                            .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f },
                            .initialLayout = TextureLayout::ColorAttachmentOptimal,
                            .finalLayout = TextureLayout::PresentSrc,
                    },
            },
            .depthStencilAttachment = {
                    .view = m_depthTextureView,
                    .depthLoadOperation = AttachmentLoadOperation::Load,
                    .stencilLoadOperation = AttachmentLoadOperation::Load,
                    .initialLayout = TextureLayout::DepthStencilAttachmentOptimal,
            },
        };
    };
    initializeGraphicsPipeline();
}

void HybridRasterRt::initializeMeshes()
{
    auto initializeSphereBuffers = [this]() -> void {
        const std::vector<Vertex> vertices = initializeSphereMesh();
        m_sphereMesh.vertexBuffer = m_device.createBuffer(BufferOptions{
                                                                  .size = vertices.size() * sizeof(Vertex),
                                                                  .usage = BufferUsageFlagBits::VertexBufferBit | BufferUsageFlagBits::ShaderDeviceAddressBit | BufferUsageFlagBits::AccelerationStructureBuildInputReadOnlyBit,
                                                                  .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
                                                          },
                                                          vertices.data());
        m_sphereMesh.vertexCount = vertices.size();
    };
    initializeSphereBuffers();

    auto initializePlaneBuffers = [this]() -> void {
        const std::vector<Vertex> vertices = initializePlaneMesh();
        m_planeMesh.vertexBuffer = m_device.createBuffer(BufferOptions{
                                                                 .size = vertices.size() * sizeof(Vertex),
                                                                 .usage = BufferUsageFlagBits::VertexBufferBit | BufferUsageFlagBits::ShaderDeviceAddressBit | BufferUsageFlagBits::AccelerationStructureBuildInputReadOnlyBit,
                                                                 .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
                                                         },
                                                         vertices.data());
        m_planeMesh.vertexCount = vertices.size();
    };
    initializePlaneBuffers();

    auto initializeSphereMeshPipeline = [this]() -> void {
        // Create a vertex shader and fragment shader (spir-v only for now)
        auto vertexShaderPath = KDGpuExample::assetDir().file("shaders/examples/hybrid_raster_rt/sphere_instanced.vert.spv");
        auto vertexShader = m_device.createShaderModule(KDGpuExample::readShaderFile(vertexShaderPath));

        auto zFillFragmentShaderPath = KDGpuExample::assetDir().file("shaders/examples/hybrid_raster_rt/zfill.frag.spv");
        auto zFillFragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(zFillFragmentShaderPath));

        auto opaqueFragmentShaderPath = KDGpuExample::assetDir().file("shaders/examples/hybrid_raster_rt/opaque.frag.spv");
        auto opaqueFragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(opaqueFragmentShaderPath));

        auto alphaFragmentShaderPath = KDGpuExample::assetDir().file("shaders/examples/hybrid_raster_rt/alpha.frag.spv");
        auto alphaFragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(alphaFragmentShaderPath));

        // Create pipeline layouts (array of bind group layouts)
        m_sphereMesh.zFillGraphicsPipelineLayout = m_device.createPipelineLayout(PipelineLayoutOptions{
                .bindGroupLayouts = { m_global.cameraBindGroupLayout },
        });

        m_sphereMesh.alphaFillGraphicsPipelineLayout = m_device.createPipelineLayout(PipelineLayoutOptions{
                .bindGroupLayouts = { m_global.cameraBindGroupLayout, m_gbuffer.alphaBindGroupLayout },
                .pushConstantRanges = { m_global.lightPosPushConstant },
        });

        m_sphereMesh.opaqueFillGraphicsPipelineLayout = m_device.createPipelineLayout(PipelineLayoutOptions{
                .bindGroupLayouts = { m_global.cameraBindGroupLayout },
                .pushConstantRanges = { m_global.lightPosPushConstant },
        });

        // Create pipelines
        m_sphereMesh.zFillGraphicsPipeline = m_device.createGraphicsPipeline(GraphicsPipelineOptions{
                .label = "SphereZFill",
                .shaderStages = {
                        { .shaderModule = vertexShader, .stage = ShaderStageFlagBits::VertexBit },
                        { .shaderModule = zFillFragmentShader, .stage = ShaderStageFlagBits::FragmentBit } },
                .layout = m_sphereMesh.zFillGraphicsPipelineLayout,
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
                        .format = KDGpu::Format::D32_SFLOAT,
                        .depthTestEnabled = true,
                        .depthWritesEnabled = true,
                        .depthCompareOperation = CompareOperation::Less,
                },
                .primitive = {
                        .cullMode = CullModeFlagBits::BackBit,
                },
        });

        m_sphereMesh.alphaFillGraphicsPipeline = m_device.createGraphicsPipeline(GraphicsPipelineOptions{
                .label = "SphereAlpha",
                .shaderStages = {
                        { .shaderModule = vertexShader, .stage = ShaderStageFlagBits::VertexBit },
                        { .shaderModule = alphaFragmentShader, .stage = ShaderStageFlagBits::FragmentBit } },
                .layout = m_sphereMesh.alphaFillGraphicsPipelineLayout,
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
                        .format = KDGpu::Format::D32_SFLOAT,
                        .depthTestEnabled = true,
                        .depthWritesEnabled = false,
                        .depthCompareOperation = CompareOperation::LessOrEqual,
                },
                .primitive = {
                        .cullMode = CullModeFlagBits::BackBit,
                },
        });

        m_sphereMesh.opaqueFillGraphicsPipeline = m_device.createGraphicsPipeline(GraphicsPipelineOptions{
                .label = "SphereOpaque",
                .shaderStages = {
                        { .shaderModule = vertexShader, .stage = ShaderStageFlagBits::VertexBit },
                        { .shaderModule = opaqueFragmentShader, .stage = ShaderStageFlagBits::FragmentBit } },
                .layout = m_sphereMesh.opaqueFillGraphicsPipelineLayout,
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
                .renderTargets = {
                        { .format = KDGpu::Format::R32G32B32A32_SFLOAT },
                        { .format = KDGpu::Format::R32G32B32A32_SFLOAT },
                        { .format = KDGpu::Format::R32G32B32A32_SFLOAT },
                },
                .depthStencil = {
                        .format = KDGpu::Format::D32_SFLOAT,
                        .depthTestEnabled = true,
                        .depthWritesEnabled = false,
                        .depthCompareOperation = CompareOperation::Equal,
                },
                .primitive = {
                        .cullMode = CullModeFlagBits::BackBit,
                },
        });
    };
    initializeSphereMeshPipeline();

    auto initializePlaneMeshPipeline = [this]() -> void {
        // Create a vertex shader and fragment shader (spir-v only for now)
        auto vertexShaderPath = KDGpuExample::assetDir().file("shaders/examples/hybrid_raster_rt/plane.vert.spv");
        auto vertexShader = m_device.createShaderModule(KDGpuExample::readShaderFile(vertexShaderPath));

        auto zFillFragmentShaderPath = KDGpuExample::assetDir().file("shaders/examples/hybrid_raster_rt/zfill.frag.spv");
        auto zFillFragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(zFillFragmentShaderPath));

        auto opaqueFragmentShaderPath = KDGpuExample::assetDir().file("shaders/examples/hybrid_raster_rt/opaque.frag.spv");
        auto opaqueFragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(opaqueFragmentShaderPath));

        // Create a pipeline layout (array of bind group layouts)
        m_planeMesh.zFillGraphicsPipelineLayout = m_device.createPipelineLayout(PipelineLayoutOptions{
                .bindGroupLayouts = { m_global.cameraBindGroupLayout },
        });
        m_planeMesh.opaqueFillGraphicsPipelineLayout = m_device.createPipelineLayout(PipelineLayoutOptions{
                .bindGroupLayouts = { m_global.cameraBindGroupLayout },
                .pushConstantRanges = { m_global.lightPosPushConstant },
        });

        // Create a pipeline
        m_planeMesh.zFillGraphicsPipeline = m_device.createGraphicsPipeline(GraphicsPipelineOptions{
                .label = "PlaneZFill",
                .shaderStages = {
                        { .shaderModule = vertexShader, .stage = ShaderStageFlagBits::VertexBit },
                        { .shaderModule = zFillFragmentShader, .stage = ShaderStageFlagBits::FragmentBit } },
                .layout = m_planeMesh.zFillGraphicsPipelineLayout,
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
                        .format = KDGpu::Format::D32_SFLOAT,
                        .depthTestEnabled = true,
                        .depthWritesEnabled = true,
                        .depthCompareOperation = CompareOperation::Less,
                },
                .primitive = {
                        .cullMode = CullModeFlagBits::None,
                },
        });

        m_planeMesh.opaqueFillGraphicsPipeline = m_device.createGraphicsPipeline(GraphicsPipelineOptions{
                .label = "PlaneOpaqueFill",
                .shaderStages = {
                        { .shaderModule = vertexShader, .stage = ShaderStageFlagBits::VertexBit },
                        { .shaderModule = opaqueFragmentShader, .stage = ShaderStageFlagBits::FragmentBit } },
                .layout = m_planeMesh.opaqueFillGraphicsPipelineLayout,
                .vertex = {
                        .buffers = {
                                { .binding = 0, .stride = sizeof(Vertex) },
                        },
                        .attributes = {
                                { .location = 0, .binding = 0, .format = Format::R32G32B32_SFLOAT }, // Vertex Position
                                { .location = 1, .binding = 0, .format = Format::R32G32B32_SFLOAT, .offset = sizeof(glm::vec3) }, // Vertex Normal
                        },
                },
                .renderTargets = {
                        { .format = KDGpu::Format::R32G32B32A32_SFLOAT },
                        { .format = KDGpu::Format::R32G32B32A32_SFLOAT },
                        { .format = KDGpu::Format::R32G32B32A32_SFLOAT },
                },
                .depthStencil = {
                        .format = KDGpu::Format::D32_SFLOAT,
                        .depthTestEnabled = true,
                        .depthWritesEnabled = false,
                        .depthCompareOperation = CompareOperation::Equal,
                },
                .primitive = {
                        .cullMode = CullModeFlagBits::None,
                },
        });
    };
    initializePlaneMeshPipeline();
}

void HybridRasterRt::initializeAccelerationStructures()
{
    auto initializeSphereBLAS = [this] {
        m_as.opaqueSpheresBlas = m_device.createAccelerationStructure(AccelerationStructureOptions{
                .label = "OpaqueSphereBLAS",
                .type = AccelerationStructureType::BottomLevel,
                .flags = AccelerationStructureFlagBits::PreferFastBuild,
                .geometryTypesAndCount = std::vector<AccelerationStructureOptions::GeometryTypeAndCount>(
                        OpaqueSpheresCount,
                        {
                                .geometry = AccelerationStructureGeometryTrianglesData{
                                        .vertexFormat = Format::R32G32B32_SFLOAT,
                                        .vertexStride = sizeof(Vertex),
                                        .maxVertex = static_cast<uint32_t>(m_sphereMesh.vertexCount - 1),
                                },
                                .maxPrimitiveCount = static_cast<uint32_t>(m_sphereMesh.vertexCount / 3),
                        }),
        });
        m_as.alphaSpheresBlas = m_device.createAccelerationStructure(AccelerationStructureOptions{
                .label = "AlphaSphereBLAS",
                .type = AccelerationStructureType::BottomLevel,
                .flags = AccelerationStructureFlagBits::PreferFastBuild,
                .geometryTypesAndCount = std::vector<AccelerationStructureOptions::GeometryTypeAndCount>(
                        AlphaSpheresCount,
                        {
                                .geometry = AccelerationStructureGeometryTrianglesData{
                                        .vertexFormat = Format::R32G32B32_SFLOAT,
                                        .vertexStride = sizeof(Vertex),
                                        .maxVertex = static_cast<uint32_t>(m_sphereMesh.vertexCount - 1),
                                },
                                .maxPrimitiveCount = static_cast<uint32_t>(m_sphereMesh.vertexCount / 3),
                        }),
        });

        auto buildSphereTriangleGeometries = [this](Handle<AccelerationStructure_t> dstStructure,
                                                    size_t count, size_t transformOffset = 0) {
            std::vector<AccelerationStructureGeometry> geometries;
            geometries.reserve(count);

            for (size_t i = 0; i < count; ++i) {
                geometries.emplace_back(
                        AccelerationStructureGeometryTrianglesData{
                                .vertexFormat = Format::R32G32B32_SFLOAT,
                                .vertexData = m_sphereMesh.vertexBuffer,
                                .vertexStride = sizeof(Vertex),
                                .maxVertex = static_cast<uint32_t>(m_sphereMesh.vertexCount - 1),
                                .transformData = m_particles.blasTransformBuffer,
                                .transformDataOffset = transformOffset + i * sizeof(VkTransformMatrixKHR),
                        });
            }
            return BuildAccelerationStructureOptions::BuildOptions{
                .geometries = std::move(geometries),
                .destinationStructure = dstStructure,
                .buildRangeInfos = std::vector<BuildAccelerationStructureOptions::BuildRangeInfo>(
                        count,
                        {
                                .primitiveCount = static_cast<uint32_t>(m_sphereMesh.vertexCount / 3),
                        }),
            };
        };

        m_as.opaqueSpheresASBuildOptions = {
            .buildGeometryInfos = { buildSphereTriangleGeometries(m_as.opaqueSpheresBlas, OpaqueSpheresCount, 0) },
        };

        m_as.alphaSpheresASBuildOptions = {
            .buildGeometryInfos = { buildSphereTriangleGeometries(m_as.alphaSpheresBlas, AlphaSpheresCount, OpaqueSpheresCount * sizeof(VkTransformMatrixKHR)) },
        };
    };
    initializeSphereBLAS();

    auto initializePlaneBLAS = [this] {
        m_as.opaquePlaneBlas = m_device.createAccelerationStructure(AccelerationStructureOptions{
                .label = "PlaneBLAS",
                .type = AccelerationStructureType::BottomLevel,
                .flags = AccelerationStructureFlagBits::PreferFastBuild,
                .geometryTypesAndCount = {
                        {
                                .geometry = AccelerationStructureGeometryTrianglesData{
                                        .vertexFormat = Format::R32G32B32_SFLOAT,
                                        .vertexStride = sizeof(Vertex),
                                        .maxVertex = static_cast<uint32_t>(m_planeMesh.vertexCount - 1),
                                },
                                .maxPrimitiveCount = static_cast<uint32_t>(m_planeMesh.vertexCount / 3),
                        },
                },
        });

        m_as.opaquePlaneASBuildOptions = {
            .buildGeometryInfos = {
                    {
                            .geometries = {
                                    AccelerationStructureGeometryTrianglesData{
                                            .vertexFormat = Format::R32G32B32_SFLOAT,
                                            .vertexData = m_planeMesh.vertexBuffer,
                                            .vertexStride = sizeof(Vertex),
                                            .maxVertex = static_cast<uint32_t>(m_planeMesh.vertexCount - 1),
                                    },
                            },
                            .destinationStructure = m_as.opaquePlaneBlas,
                            .buildRangeInfos = {
                                    { .primitiveCount = static_cast<uint32_t>(m_planeMesh.vertexCount / 3) },
                            },
                    },
            },
        };
    };
    initializePlaneBLAS();

    auto initialTBLAS = [this] {
        m_as.tBlas = m_device.createAccelerationStructure(AccelerationStructureOptions{
                .label = "TBLAS",
                .type = AccelerationStructureType::TopLevel,
                .flags = AccelerationStructureFlagBits::PreferFastBuild,
                .geometryTypesAndCount = {
                        {
                                .geometry = AccelerationStructureGeometryInstancesData{},
                                .maxPrimitiveCount = 3, // 3 BLAS
                        },
                },
        });

        m_as.tlASBuildOptions = {
            .buildGeometryInfos = {
                    {
                            .geometries = {
                                    AccelerationStructureGeometryInstancesData{
                                            .data = {
                                                    AccelerationStructureGeometryInstance{
                                                            .flags = GeometryInstanceFlagBits::TriangleFacingCullDisable | GeometryInstanceFlagBits::ForceOpaque,
                                                            .accelerationStructure = m_as.opaqueSpheresBlas,
                                                    },
                                                    AccelerationStructureGeometryInstance{
                                                            .flags = GeometryInstanceFlagBits::TriangleFacingCullDisable | GeometryInstanceFlagBits::ForceNoOpaque,
                                                            .accelerationStructure = m_as.alphaSpheresBlas,
                                                    },
                                                    AccelerationStructureGeometryInstance{
                                                            .flags = GeometryInstanceFlagBits::TriangleFacingCullDisable | GeometryInstanceFlagBits::ForceOpaque,
                                                            .accelerationStructure = m_as.opaquePlaneBlas,
                                                    },
                                            },
                                    },
                            },
                            .destinationStructure = m_as.tBlas,
                            .buildRangeInfos = {
                                    { .primitiveCount = 3 }, // 3 BLAS
                            },
                    },
            },
        };
    };
    initialTBLAS();

    // Create bind group layout to hold acceleration structure
    m_as.tsASBindGroupLayout = m_device.createBindGroupLayout(BindGroupLayoutOptions{
            .bindings = {
                    {
                            // Acceleration Structure
                            .binding = 0,
                            .count = 1,
                            .resourceType = ResourceBindingType::AccelerationStructure,
                            .shaderStages = ShaderStageFlags(ShaderStageFlagBits::RaygenBit),
                    },
            },
    });
    m_as.tsASBindGroup = m_device.createBindGroup({
            .label = "RT Shadow Bind Group",
            .layout = m_as.tsASBindGroupLayout,
            .resources = {
                    {
                            .binding = 0,
                            .resource = AccelerationStructureBinding{
                                    .accelerationStructure = m_as.tBlas,
                            },
                    },
            },
    });
}

void HybridRasterRt::initializeScene()
{
    initializeGlobal();
    initializeParticles();
    initializeGBuffer();
    initializeAlpha();
    initializeCompositing();
    initializeLightDisplay();
    initializeMeshes();
    initializeAccelerationStructures();
    initializeShadows();

    resize();
}

void HybridRasterRt::cleanupScene()
{
    m_gbuffer.cleanup();

    m_particles = {};
    m_alphaPass = {};
    m_compositing = {};
    m_lightDisplayPass = {};
    m_shadowPass = {};
    m_planeMesh = {};
    m_sphereMesh = {};
    m_as = {};
    m_global = {};
}

void HybridRasterRt::updateScene()
{
    // Update Light Pos
    static float step = 0.0f;
    step += 0.001f;
    m_global.lightPos = glm::vec3(-60.0 * std::cos(step), -60.0 * std::fabs(std::sin(step)), 60.0 * std::cos(step));
}

void HybridRasterRt::resize()
{
    // Recreated GBuffer textures
    m_gbuffer.resize(&m_device, m_swapchainExtent);

    // Swapchain might have been resized and texture views recreated. Ensure we update the PassOptions accordingly
    m_compositing.renderPassOptions.depthStencilAttachment.view = m_depthTextureView;
    m_lightDisplayPass.renderPassOptions.depthStencilAttachment.view = m_depthTextureView;

    // Specify framebuffer dimensions when it cannot be deduced from the color attachments
    m_zfillPass.renderPassOptions = RenderPassCommandRecorderOptions{
        .colorAttachments = {},
        .depthStencilAttachment = { .view = m_gbuffer.depthTextureView },
        .framebufferWidth = m_swapchainExtent.width,
        .framebufferHeight = m_swapchainExtent.height,
        .framebufferArrayLayers = 1,
    };

    m_opaquePass.renderPassOptions = RenderPassCommandRecorderOptions{
        .colorAttachments = {
                { .view = m_gbuffer.posTextureView, .clearValue = { .float32 = { 0.0f, 0.0f, 0.0f, 0.0f } } },
                { .view = m_gbuffer.normalTextureView, .clearValue = { .float32 = { 0.0f, 0.0f, 0.0f, 0.0f } } },
                { .view = m_gbuffer.colorTextureView, .clearValue = { .float32 = { 0.2f, 0.2f, 0.2f, 1.0f } } },
        },
        .depthStencilAttachment = {
                .view = m_gbuffer.depthTextureView,
                .depthLoadOperation = AttachmentLoadOperation::Load,
                .initialLayout = TextureLayout::DepthStencilAttachmentOptimal,
        },
    };

    // Specify framebuffer dimensions when it cannot be deduced from the color attachments
    m_alphaPass.renderPassOptions = RenderPassCommandRecorderOptions{
        .colorAttachments = {},
        .depthStencilAttachment = {
                .view = m_gbuffer.depthTextureView,
                .depthLoadOperation = AttachmentLoadOperation::Load,
                .initialLayout = TextureLayout::DepthStencilAttachmentOptimal,
        },
        .framebufferWidth = m_swapchainExtent.width,
        .framebufferHeight = m_swapchainExtent.height,
        .framebufferArrayLayers = 1,
    };

    void *cameraData = m_global.cameraDataBuffer.map();
    const std::vector<uint8_t> rawCameraData = updateCameraData(m_window->width(), std::max(m_window->height(), uint32_t(1)));
    std::memcpy(cameraData, rawCameraData.data(), 2 * sizeof(glm::mat4));
    m_global.cameraDataBuffer.unmap();
}

void HybridRasterRt::render()
{
    auto commandRecorder = m_device.createCommandRecorder();
    {
        // 1) We use a compute shader to update particles positions / BLAS transform data
        {
            commandRecorder.beginDebugLabel(DebugLabelOptions{
                    .label = "Compute - Particles Update",
                    .color = { 0.0f, 1.0f, 0.0f, 1.0f },
            });

            auto computePass = commandRecorder.beginComputePass();
            computePass.setPipeline(m_particles.computePipeline);
            computePass.setBindGroup(0, m_particles.particleBindGroup);
            constexpr size_t LocalWorkGroupXSize = 256;
            computePass.dispatchCompute(ComputeCommand{ .workGroupX = ParticlesCount / LocalWorkGroupXSize + 1 });
            computePass.end();

            commandRecorder.endDebugLabel();
        }

        // 2) We schedule BLAS rebuild
        // Build acceleration structures from updated Particles Transforms
        {
            commandRecorder.beginDebugLabel(DebugLabelOptions{
                    .label = "Acceleration Structures Rebuild",
                    .color = { 1.0f, 0.0f, 0.0f, 1.0f },
            });

            commandRecorder.bufferMemoryBarrier(KDGpu::BufferMemoryBarrierOptions{
                    .srcStages = KDGpu::PipelineStageFlagBit::ComputeShaderBit,
                    .srcMask = KDGpu::AccessFlagBit::ShaderWriteBit,
                    .dstStages = KDGpu::PipelineStageFlagBit::AccelerationStructureBuildBit,
                    .dstMask = KDGpu::AccessFlagBit::AccelerationStructureReadBit,
                    .buffer = m_particles.blasTransformBuffer,
            });

            commandRecorder.buildAccelerationStructures(m_as.opaqueSpheresASBuildOptions);
            commandRecorder.buildAccelerationStructures(m_as.alphaSpheresASBuildOptions);

            if (!m_as.hasBuiltStaticBlas) {
                // Only needs to be done once
                commandRecorder.buildAccelerationStructures(m_as.opaquePlaneASBuildOptions);
                m_as.hasBuiltStaticBlas = true;
            }

            // Wait for the BLAS to have been built prior to building the TLAS
            commandRecorder.memoryBarrier(MemoryBarrierOptions{
                    .srcStages = PipelineStageFlags(PipelineStageFlagBit::AccelerationStructureBuildBit),
                    .dstStages = PipelineStageFlags(PipelineStageFlagBit::AccelerationStructureBuildBit),
                    .memoryBarriers = {
                            {
                                    .srcMask = AccessFlags(AccessFlagBit::AccelerationStructureWriteBit),
                                    .dstMask = AccessFlags(AccessFlagBit::AccelerationStructureReadBit),
                            },
                    },
            });

            commandRecorder.buildAccelerationStructures(m_as.tlASBuildOptions);

            commandRecorder.endDebugLabel();
        }

        // 3) GBuffer fill
        {

            // Wait for SSBO writes completion by ComputeShader
            commandRecorder.bufferMemoryBarrier(KDGpu::BufferMemoryBarrierOptions{
                    .srcStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::ComputeShaderBit),
                    .srcMask = KDGpu::AccessFlagBit::ShaderWriteBit,
                    .dstStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::VertexInputBit),
                    .dstMask = KDGpu::AccessFlagBit::VertexAttributeReadBit,
                    .buffer = m_particles.particleDataBuffer,
            });

            // Depth Fill from Opaque Content
            {
                commandRecorder.beginDebugLabel(DebugLabelOptions{
                        .label = "GBuffer Depth Fill",
                        .color = { 0.0f, 1.0f, 0.0f, 1.0f },
                });

                auto opaquePass = commandRecorder.beginRenderPass(m_zfillPass.renderPassOptions);

                // Draw Opaque Spheres
                opaquePass.setPipeline(m_sphereMesh.zFillGraphicsPipeline);
                opaquePass.setBindGroup(0, m_global.cameraBindGroup);
                opaquePass.setVertexBuffer(0, m_sphereMesh.vertexBuffer);
                opaquePass.setVertexBuffer(1, m_particles.particleDataBuffer); // Per instance Data
                opaquePass.draw(DrawCommand{ .vertexCount = uint32_t(m_sphereMesh.vertexCount), .instanceCount = OpaqueSpheresCount });

                // Draw Plane
                opaquePass.setPipeline(m_planeMesh.zFillGraphicsPipeline);
                opaquePass.setBindGroup(0, m_global.cameraBindGroup);
                opaquePass.setVertexBuffer(0, m_planeMesh.vertexBuffer);
                opaquePass.draw(DrawCommand{ .vertexCount = uint32_t(m_planeMesh.vertexCount), .instanceCount = 1 });

                opaquePass.end();

                commandRecorder.endDebugLabel();
            }

            // Alpha Fill OIT Linked List
            {
                commandRecorder.beginDebugLabel(DebugLabelOptions{
                        .label = "GBuffer Alpha OIT Fill",
                        .color = { 1.0f, 1.0f, 0.0f, 1.0f },
                });

                // Clear Fragment List SSBO
                commandRecorder.clearBuffer(KDGpu::BufferClear{
                        .dstBuffer = m_gbuffer.fragmentLinkedListBuffer,
                        .byteSize = m_gbuffer.fragmentLinkedListBufferByteSize,
                });

                // Transition fragmentHeadsPointer to general layout if needed
                if (m_gbuffer.fragmentHeadsPointerLayout == KDGpu::TextureLayout::Undefined) {
                    commandRecorder.textureMemoryBarrier(KDGpu::TextureMemoryBarrierOptions{
                            .srcStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::TopOfPipeBit),
                            .srcMask = KDGpu::AccessFlagBit::None,
                            .dstStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::TransferBit),
                            .dstMask =
                                    KDGpu::AccessFlagBit::TransferWriteBit | KDGpu::AccessFlagBit::TransferReadBit,
                            .oldLayout = KDGpu::TextureLayout::Undefined,
                            .newLayout = KDGpu::TextureLayout::General,
                            .texture = m_gbuffer.fragmentHeadsPointer,
                            .range = {
                                    .aspectMask = KDGpu::TextureAspectFlagBits::ColorBit,
                                    .levelCount = 1,
                            },
                    });
                    m_gbuffer.fragmentHeadsPointerLayout = KDGpu::TextureLayout::General;
                }

                // Clear Fragment Head Texture Image
                commandRecorder.clearColorTexture(KDGpu::ClearColorTexture{
                        .texture = m_gbuffer.fragmentHeadsPointer,
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
                        .buffer = m_gbuffer.fragmentLinkedListBuffer,
                });

                // Wait until fragments SSBO Heads pointer image has been cleared
                commandRecorder.textureMemoryBarrier(KDGpu::TextureMemoryBarrierOptions{
                        .srcStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::TransferBit),
                        .srcMask = KDGpu::AccessFlagBit::TransferWriteBit,
                        .dstStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::FragmentShaderBit),
                        .dstMask = KDGpu::AccessFlagBit::ShaderWriteBit | KDGpu::AccessFlagBit::ShaderReadBit,
                        .oldLayout = KDGpu::TextureLayout::General,
                        .newLayout = KDGpu::TextureLayout::General,
                        .texture = m_gbuffer.fragmentHeadsPointer,
                        .range = {
                                .aspectMask = KDGpu::TextureAspectFlagBits::ColorBit,
                                .levelCount = 1,
                        },
                });

                // Wait until depth buffer has been filled (implicit since referenced by the RenderPass)

                // Render Alpha meshes to fragment list
                auto alphaPass = commandRecorder.beginRenderPass(m_alphaPass.renderPassOptions);

                // Draw Alpha Spheres
                alphaPass.setPipeline(m_sphereMesh.alphaFillGraphicsPipeline);
                alphaPass.setBindGroup(0, m_global.cameraBindGroup);
                alphaPass.setBindGroup(1, m_gbuffer.alphaLinkedListBindGroup);
                alphaPass.setVertexBuffer(0, m_sphereMesh.vertexBuffer);
                alphaPass.setVertexBuffer(1, m_particles.particleDataBuffer); // Per instance Data
                alphaPass.pushConstant(m_global.lightPosPushConstant, &m_global.lightPos);
                alphaPass.draw(DrawCommand{ .vertexCount = uint32_t(m_sphereMesh.vertexCount), .instanceCount = AlphaSpheresCount, .firstInstance = OpaqueSpheresCount });

                alphaPass.end();

                commandRecorder.endDebugLabel();
            }

            // Opaque GBuffer Fill
            {
                commandRecorder.beginDebugLabel(DebugLabelOptions{
                        .label = "GBuffer Opaque Fill",
                        .color = { 0.0f, 1.0f, 1.0f, 1.0f },
                });

                auto opaquePass = commandRecorder.beginRenderPass(m_opaquePass.renderPassOptions);

                // Draw Opaque Spheres
                opaquePass.setPipeline(m_sphereMesh.opaqueFillGraphicsPipeline);
                opaquePass.pushConstant(m_global.lightPosPushConstant, &m_global.lightPos);
                opaquePass.setBindGroup(0, m_global.cameraBindGroup);
                opaquePass.setVertexBuffer(0, m_sphereMesh.vertexBuffer);
                opaquePass.setVertexBuffer(1, m_particles.particleDataBuffer); // Per instance Data
                opaquePass.draw(DrawCommand{ .vertexCount = uint32_t(m_sphereMesh.vertexCount), .instanceCount = OpaqueSpheresCount });

                // Draw Plane
                opaquePass.setPipeline(m_planeMesh.opaqueFillGraphicsPipeline);
                opaquePass.setBindGroup(0, m_global.cameraBindGroup);
                opaquePass.setVertexBuffer(0, m_planeMesh.vertexBuffer);
                opaquePass.draw(DrawCommand{ .vertexCount = 36, .instanceCount = 1 });

                opaquePass.end();

                commandRecorder.endDebugLabel();
            }
        }

        // 5) Shadow Raytracing Pass
        // Await BLAS rebuild completion and opaque GBuffer fill to issue RT shadow pass
        {
            commandRecorder.beginDebugLabel(DebugLabelOptions{
                    .label = "Shadow RT",
                    .color = { 0.5f, 1.0f, 0.5f, 1.0f },
            });

            // Await TLAS rebuild
            commandRecorder.memoryBarrier(MemoryBarrierOptions{
                    .srcStages = PipelineStageFlags(PipelineStageFlagBit::AccelerationStructureBuildBit),
                    .dstStages = PipelineStageFlags(PipelineStageFlagBit::RayTracingShaderBit),
                    .memoryBarriers = {
                            {
                                    .srcMask = AccessFlagBit::AccelerationStructureWriteBit,
                                    .dstMask = AccessFlagBit::AccelerationStructureReadBit,
                            },
                    },
            });

            // Await GBuffer World Pos filling
            commandRecorder.textureMemoryBarrier(KDGpu::TextureMemoryBarrierOptions{
                    .srcStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::ColorAttachmentOutputBit),
                    .srcMask = KDGpu::AccessFlagBit::ColorAttachmentWriteBit,
                    .dstStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::RayTracingShaderBit),
                    .dstMask = KDGpu::AccessFlagBit::ShaderReadBit,
                    .oldLayout = KDGpu::TextureLayout::ColorAttachmentOptimal,
                    .newLayout = KDGpu::TextureLayout::ShaderReadOnlyOptimal,
                    .texture = m_gbuffer.posTexture,
                    .range = {
                            .aspectMask = KDGpu::TextureAspectFlagBits::ColorBit,
                            .levelCount = 1,
                    },
            });

            // Transition Shadow Image to General Layout
            commandRecorder.textureMemoryBarrier(TextureMemoryBarrierOptions{
                    .srcStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::TopOfPipeBit),
                    .srcMask = KDGpu::AccessFlagBit::None,
                    .dstStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::RayTracingShaderBit),
                    .dstMask = KDGpu::AccessFlagBit::ShaderStorageReadBit | KDGpu::AccessFlagBit::ShaderStorageWriteBit,
                    .oldLayout = m_gbuffer.shadowTextureLayout,
                    .newLayout = KDGpu::TextureLayout::General,
                    .texture = m_gbuffer.shadowTexture,
                    .range = {
                            .aspectMask = KDGpu::TextureAspectFlagBits::ColorBit,
                            .levelCount = 1,
                    },
            });
            m_gbuffer.shadowTextureLayout = KDGpu::TextureLayout::General;

            auto rtPass = commandRecorder.beginRayTracingPass();
            rtPass.setPipeline(m_shadowPass.pipeline);
            rtPass.pushConstant(m_global.lightPosPushConstant, &m_global.lightPos);
            rtPass.setBindGroup(0, m_gbuffer.opaqueNormalDepthBindGroup);
            rtPass.setBindGroup(1, m_gbuffer.shadowBindGroup);
            rtPass.setBindGroup(2, m_as.tsASBindGroup);

            // Issue RT Trace call using the SBT table we previously filled

            // Do note one thing:
            // Opaque BLAS will use closest hit shader (the any hit shader is disabled for BLAS marked opaque)
            // ALPHA BLAS will use the any hit shader

            rtPass.traceRays(RayTracingCommand{
                    .raygenShaderBindingTable = m_shadowPass.sbt.rayGenShaderRegion(),
                    .missShaderBindingTable = m_shadowPass.sbt.missShaderRegion(),
                    .hitShaderBindingTable = m_shadowPass.sbt.hitShaderRegion(),
                    .extent = {
                            .width = m_swapchainExtent.width,
                            .height = m_swapchainExtent.height,
                            .depth = 1,
                    },
            });
            rtPass.end();

            commandRecorder.endDebugLabel();
        }

        // 6) Compositing
        {
            commandRecorder.beginDebugLabel(DebugLabelOptions{
                    .label = "Compositing",
                    .color = { 1.0f, 1.0f, 1.0f, 1.0f },
            });

            commandRecorder.memoryBarrier(MemoryBarrierOptions{
                    .srcStages = PipelineStageFlags(PipelineStageFlagBit::RayTracingShaderBit),
                    .dstStages = PipelineStageFlags(PipelineStageFlagBit::TopOfPipeBit),
                    .memoryBarriers = {},
            });

            // Wait until shadows have been filled
            commandRecorder.textureMemoryBarrier(KDGpu::TextureMemoryBarrierOptions{
                    .srcStages = KDGpu::PipelineStageFlagBit::RayTracingShaderBit,
                    .srcMask = KDGpu::AccessFlagBit::ShaderStorageWriteBit,
                    .dstStages = KDGpu::PipelineStageFlagBit::FragmentShaderBit,
                    .dstMask = KDGpu::AccessFlagBit::ShaderStorageReadBit,
                    .oldLayout = KDGpu::TextureLayout::General,
                    .newLayout = KDGpu::TextureLayout::General,
                    .texture = m_gbuffer.shadowTexture,
                    .range = {
                            .aspectMask = KDGpu::TextureAspectFlagBits::ColorBit,
                            .levelCount = 1,
                    },
            });

            // Wait until fragment Heads pointer image writes have been completed
            commandRecorder.textureMemoryBarrier(KDGpu::TextureMemoryBarrierOptions{
                    .srcStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::FragmentShaderBit),
                    .srcMask = KDGpu::AccessFlagBit::ShaderWriteBit,
                    .dstStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::FragmentShaderBit),
                    .dstMask = KDGpu::AccessFlagBit::ShaderReadBit,
                    .oldLayout = KDGpu::TextureLayout::General,
                    .newLayout = KDGpu::TextureLayout::General,
                    .texture = m_gbuffer.fragmentHeadsPointer,
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
                    .buffer = m_gbuffer.fragmentLinkedListBuffer,
            });

            // Wait until Opaque Color texture has been filled
            commandRecorder.textureMemoryBarrier(KDGpu::TextureMemoryBarrierOptions{
                    .srcStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::ColorAttachmentOutputBit),
                    .srcMask = KDGpu::AccessFlagBit::ColorAttachmentWriteBit,
                    .dstStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::FragmentShaderBit),
                    .dstMask = KDGpu::AccessFlagBit::ShaderReadBit,
                    .oldLayout = KDGpu::TextureLayout::ColorAttachmentOptimal,
                    .newLayout = KDGpu::TextureLayout::ShaderReadOnlyOptimal,
                    .texture = m_gbuffer.colorTexture,
                    .range = {
                            .aspectMask = KDGpu::TextureAspectFlagBits::ColorBit,
                            .levelCount = 1,
                    },
            });

            // Render Compositing full screen quad to screen
            m_compositing.renderPassOptions.colorAttachments[0].view = m_swapchainViews.at(m_currentSwapchainImageIndex);

            auto compositingPass = commandRecorder.beginRenderPass(m_compositing.renderPassOptions);
            compositingPass.setPipeline(m_compositing.graphicsPipeline);
            compositingPass.setBindGroup(0, m_gbuffer.opaqueNormalDepthBindGroup);
            compositingPass.setBindGroup(1, m_gbuffer.alphaLinkedListBindGroup);
            compositingPass.setBindGroup(2, m_gbuffer.shadowBindGroup);
            compositingPass.draw(DrawCommand{ .vertexCount = 6 });
            compositingPass.end();

            commandRecorder.endDebugLabel();
        }

        // 7) Display Light
        {
            commandRecorder.beginDebugLabel(DebugLabelOptions{
                    .label = "LightDisplay",
                    .color = { 1.0f, 0.5f, 0.5f, 1.0f },
            });

            m_lightDisplayPass.renderPassOptions.colorAttachments[0].view = m_swapchainViews.at(m_currentSwapchainImageIndex);

            auto lightDisplayPass = commandRecorder.beginRenderPass(m_lightDisplayPass.renderPassOptions);
            lightDisplayPass.setPipeline(m_lightDisplayPass.graphicsPipeline);
            lightDisplayPass.setBindGroup(0, m_global.cameraBindGroup);
            lightDisplayPass.pushConstant(m_global.lightPosPushConstant, &m_global.lightPos);
            lightDisplayPass.draw(DrawCommand{ .vertexCount = 8 });
            lightDisplayPass.end();

            commandRecorder.endDebugLabel();
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

void HybridRasterRt::GBuffer::initialize(Device *device)
{
    // BindGroupLayouts
    opaqueNormalDepthBindGroupLayout = device->createBindGroupLayout(BindGroupLayoutOptions{
            .bindings = {
                    {
                            // POSITIONS
                            .binding = 0,
                            .resourceType = ResourceBindingType::CombinedImageSampler,
                            .shaderStages = KDGpu::ShaderStageFlagBits::FragmentBit | KDGpu::ShaderStageFlagBits::RaygenBit,
                    },
                    {
                            // NORMALS
                            .binding = 1,
                            .resourceType = ResourceBindingType::CombinedImageSampler,
                            .shaderStages = KDGpu::ShaderStageFlagBits::FragmentBit | KDGpu::ShaderStageFlagBits::RaygenBit,
                    },
                    {
                            // COLORS
                            .binding = 2,
                            .resourceType = ResourceBindingType::CombinedImageSampler,
                            .shaderStages = KDGpu::ShaderStageFlagBits::FragmentBit | KDGpu::ShaderStageFlagBits::RaygenBit,
                    },
                    {
                            // DEPTH
                            .binding = 3,
                            .resourceType = ResourceBindingType::CombinedImageSampler,
                            .shaderStages = KDGpu::ShaderStageFlagBits::FragmentBit | KDGpu::ShaderStageFlagBits::RaygenBit,
                    } },
    });
    alphaBindGroupLayout = device->createBindGroupLayout(BindGroupLayoutOptions{
            .bindings = {
                    {
                            .binding = 0,
                            .resourceType = ResourceBindingType::StorageBuffer,
                            .shaderStages = KDGpu::ShaderStageFlagBits::FragmentBit,
                    },
                    {
                            .binding = 1,
                            .resourceType = ResourceBindingType::StorageImage,
                            .shaderStages = KDGpu::ShaderStageFlagBits::FragmentBit,
                    },
            },
    });
    shadowBindGroupLayout = device->createBindGroupLayout(BindGroupLayoutOptions{
            .bindings = {
                    {
                            .binding = 0,
                            .resourceType = ResourceBindingType::StorageImage,
                            .shaderStages = KDGpu::ShaderStageFlagBits::FragmentBit | ShaderStageFlagBits::RaygenBit,
                    },
            },
    });

    // BindGroups
    opaqueNormalDepthBindGroup = device->createBindGroup(BindGroupOptions{
            .layout = opaqueNormalDepthBindGroupLayout,
            .resources = {},
    });
    alphaLinkedListBindGroup = device->createBindGroup(BindGroupOptions{
            .layout = alphaBindGroupLayout,
            .resources = {},
    });
    shadowBindGroup = device->createBindGroup(BindGroupOptions{
            .layout = shadowBindGroupLayout,
            .resources = {},
    });

    // Sampler
    sampler = device->createSampler(SamplerOptions{});
}

void HybridRasterRt::GBuffer::cleanup()
{
    shadowBindGroup = {};
    alphaLinkedListBindGroup = {};
    opaqueNormalDepthBindGroup = {};

    shadowBindGroupLayout = {};
    alphaBindGroupLayout = {};
    opaqueNormalDepthBindGroupLayout = {};

    shadowTextureView = {};
    fragmentHeadsPointerView = {};
    depthTextureView = {};
    colorTextureView = {};
    normalTextureView = {};
    posTextureView = {};

    shadowTexture = {};
    fragmentHeadsPointer = {};
    depthTexture = {};
    colorTexture = {};
    normalTexture = {};
    posTexture = {};

    fragmentLinkedListBuffer = {};

    sampler = {};
}

void HybridRasterRt::GBuffer::resize(Device *device, Extent2D extent)
{
    // Recreate Textures
    posTexture = device->createTexture(KDGpu::TextureOptions{
            .label = "posTexture",
            .type = KDGpu::TextureType::TextureType2D,
            .format = KDGpu::Format::R32G32B32A32_SFLOAT,
            .extent = { std::max(extent.width, uint32_t(1)), std::max(extent.height, uint32_t(1)), 1 },
            .mipLevels = 1,
            .usage = KDGpu::TextureUsageFlagBits::ColorAttachmentBit | KDGpu::TextureUsageFlagBits::SampledBit,
            .memoryUsage = KDGpu::MemoryUsage::GpuOnly,
    });
    normalTexture = device->createTexture(KDGpu::TextureOptions{
            .label = "normalTexture",
            .type = KDGpu::TextureType::TextureType2D,
            .format = KDGpu::Format::R32G32B32A32_SFLOAT,
            .extent = { std::max(extent.width, uint32_t(1)), std::max(extent.height, uint32_t(1)), 1 },
            .mipLevels = 1,
            .usage = KDGpu::TextureUsageFlagBits::ColorAttachmentBit | KDGpu::TextureUsageFlagBits::SampledBit,
            .memoryUsage = KDGpu::MemoryUsage::GpuOnly,
    });
    colorTexture = device->createTexture(KDGpu::TextureOptions{
            .label = "colorTexture",
            .type = KDGpu::TextureType::TextureType2D,
            .format = KDGpu::Format::R32G32B32A32_SFLOAT,
            .extent = { std::max(extent.width, uint32_t(1)), std::max(extent.height, uint32_t(1)), 1 },
            .mipLevels = 1,
            .usage = KDGpu::TextureUsageFlagBits::ColorAttachmentBit | KDGpu::TextureUsageFlagBits::SampledBit,
            .memoryUsage = KDGpu::MemoryUsage::GpuOnly,
    });
    depthTexture = device->createTexture(KDGpu::TextureOptions{
            .label = "depthTexture",
            .type = KDGpu::TextureType::TextureType2D,
            .format = KDGpu::Format::D32_SFLOAT,
            .extent = { std::max(extent.width, uint32_t(1)), std::max(extent.height, uint32_t(1)), 1 },
            .mipLevels = 1,
            .usage = KDGpu::TextureUsageFlagBits::DepthStencilAttachmentBit | KDGpu::TextureUsageFlagBits::SampledBit,
            .memoryUsage = KDGpu::MemoryUsage::GpuOnly,
    });
    fragmentHeadsPointer = device->createTexture(KDGpu::TextureOptions{
            .label = "fragmentHeadPointers",
            .type = KDGpu::TextureType::TextureType2D,
            .format = KDGpu::Format::R32_UINT,
            .extent = { std::max(extent.width, uint32_t(1)), std::max(extent.height, uint32_t(1)), 1 },
            .mipLevels = 1,
            .usage = KDGpu::TextureUsageFlagBits::TransferDstBit | KDGpu::TextureUsageFlagBits::StorageBit,
            .memoryUsage = KDGpu::MemoryUsage::GpuOnly,
    });
    shadowTexture = device->createTexture(KDGpu::TextureOptions{
            .label = "shadowTexture",
            .type = KDGpu::TextureType::TextureType2D,
            .format = KDGpu::Format::R32_SFLOAT,
            .extent = { std::max(extent.width, uint32_t(1)), std::max(extent.height, uint32_t(1)), 1 },
            .mipLevels = 1,
            .usage = KDGpu::TextureUsageFlagBits::SampledBit | KDGpu::TextureUsageFlagBits::StorageBit,
            .memoryUsage = KDGpu::MemoryUsage::GpuOnly,
    });

    fragmentHeadsPointerLayout = TextureLayout::Undefined;
    shadowTextureLayout = KDGpu::TextureLayout::Undefined;

    // Recreate Texture Views
    posTextureView = posTexture.createView(KDGpu::TextureViewOptions{
            .label = "posTextureView",
            .range = {
                    .aspectMask = KDGpu::TextureAspectFlagBits::ColorBit,
                    .levelCount = 1,
            },
    });
    normalTextureView = normalTexture.createView(KDGpu::TextureViewOptions{
            .label = "normalTextureView",
            .range = {
                    .aspectMask = KDGpu::TextureAspectFlagBits::ColorBit,
                    .levelCount = 1,
            },
    });
    colorTextureView = colorTexture.createView(KDGpu::TextureViewOptions{
            .label = "colorTextureView",
            .range = {
                    .aspectMask = KDGpu::TextureAspectFlagBits::ColorBit,
                    .levelCount = 1,
            },
    });
    depthTextureView = depthTexture.createView(KDGpu::TextureViewOptions{
            .label = "depthTextureView",
            .range = {
                    .aspectMask = KDGpu::TextureAspectFlagBits::DepthBit,
                    .levelCount = 1,
            },
    });
    fragmentHeadsPointerView = fragmentHeadsPointer.createView(KDGpu::TextureViewOptions{
            .label = "fragmentHeadPointersView",
            .range = {
                    .aspectMask = KDGpu::TextureAspectFlagBits::ColorBit,
                    .levelCount = 1,
            },
    });
    shadowTextureView = shadowTexture.createView(KDGpu::TextureViewOptions{
            .label = "shadowTextureView",
            .range = {
                    .aspectMask = KDGpu::TextureAspectFlagBits::ColorBit,
                    .levelCount = 1,
            },
    });

    // Recreate fragmentsLinkedList SSBO
    const size_t MaxFragmentCount = std::max(extent.width, uint32_t(1)) * std::max(extent.height, uint32_t(1)) * 8;

    struct FragmentInfo {
        glm::vec4 color;
        float depth;
        int32_t next;
        float _pad[2];
    };
    static_assert(sizeof(FragmentInfo) == 8 * sizeof(float));

    // vec4 to hold nextId + array of structs
    fragmentLinkedListBufferByteSize = sizeof(float) * 4 + MaxFragmentCount * sizeof(FragmentInfo);
    fragmentLinkedListBuffer = device->createBuffer(KDGpu::BufferOptions{
            .label = "FragmentSSBO",
            .size = fragmentLinkedListBufferByteSize,
            .usage = KDGpu::BufferUsageFlagBits::StorageBufferBit |
                    KDGpu::BufferUsageFlagBits::TransferDstBit,
            .memoryUsage = KDGpu::MemoryUsage::GpuOnly,
    });

    // Update Bind Groups
    opaqueNormalDepthBindGroup.update(KDGpu::BindGroupEntry{
            .binding = 0,
            .resource =
                    KDGpu::TextureViewSamplerBinding{
                            .textureView = posTextureView,
                            .sampler = sampler,
                    },
    });
    opaqueNormalDepthBindGroup.update(KDGpu::BindGroupEntry{
            .binding = 1,
            .resource =
                    KDGpu::TextureViewSamplerBinding{
                            .textureView = normalTextureView,
                            .sampler = sampler,
                    },
    });
    opaqueNormalDepthBindGroup.update(KDGpu::BindGroupEntry{
            .binding = 2,
            .resource =
                    KDGpu::TextureViewSamplerBinding{
                            .textureView = colorTextureView,
                            .sampler = sampler,
                    },
    });
    opaqueNormalDepthBindGroup.update(KDGpu::BindGroupEntry{
            .binding = 3,
            .resource =
                    KDGpu::TextureViewSamplerBinding{
                            .textureView = depthTextureView,
                            .sampler = sampler,
                    },
    });

    alphaLinkedListBindGroup.update(KDGpu::BindGroupEntry{
            .binding = 0,
            .resource =
                    KDGpu::StorageBufferBinding{
                            .buffer = fragmentLinkedListBuffer,
                    },
    });
    alphaLinkedListBindGroup.update(KDGpu::BindGroupEntry{
            .binding = 1,
            .resource =
                    KDGpu::ImageBinding{
                            .textureView = fragmentHeadsPointerView,
                    },
    });

    shadowBindGroup.update(KDGpu::BindGroupEntry{
            .binding = 0,
            .resource =
                    KDGpu::ImageBinding{
                            .textureView = shadowTextureView,
                    },
    });
}

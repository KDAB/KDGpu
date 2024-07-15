/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "hello_sphere_rt.h"

#include <KDGpuExample/engine.h>
#include <KDGpuExample/kdgpuexample.h>

#include <KDGpu/acceleration_structure.h>
#include <KDGpu/acceleration_structure_options.h>
#include <KDGpu/bind_group_layout_options.h>
#include <KDGpu/bind_group_options.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/graphics_pipeline_options.h>
#include <KDGpu/raytracing_pipeline.h>
#include <KDGpu/raytracing_pipeline_options.h>
#include <KDGpu/raytracing_pass_command_recorder.h>
#include <KDGpu/render_pass_command_recorder_options.h>
#include <KDGpu/render_pass_command_recorder.h>

#include <glm/glm.hpp>
#include <glm/ext/matrix_projection.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <fstream>
#include <string>
#include <random>

namespace KDGpu {

inline std::string assetPath()
{
#if defined(KDGPU_ASSET_PATH)
    return KDGPU_ASSET_PATH;
#else
    return "";
#endif
}

} // namespace KDGpu

namespace {

std::vector<uint8_t> updateCameraData(float width, float height, glm::vec3 position, glm::vec3 center)
{
    std::vector<uint8_t> rawCameraData(2 * sizeof(glm::mat4));
    const glm::mat4 viewMatrix = glm::lookAt(position, center, glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 projectionMatrix = glm::perspective(45.0f, width / height, 0.1f, 1000.0f);

    std::memcpy(rawCameraData.data(), glm::value_ptr(viewMatrix), sizeof(glm::mat4));
    std::memcpy(rawCameraData.data() + sizeof(glm::mat4), glm::value_ptr(projectionMatrix), sizeof(glm::mat4));

    return rawCameraData;
}

} // namespace

HelloSphereRt::HelloSphereRt()
    : SimpleExampleEngineLayer()
{
    // Request our Swapchain Images to be usable as Storage Image
    m_swapchainUsageFlags = (TextureUsageFlagBits::ColorAttachmentBit | TextureUsageFlagBits::StorageBit);
}

void HelloSphereRt::createRayTracingPipeline()
{
    // Create raytracing shaders
    const auto rayTracingGenShaderPath = KDGpu::assetPath() + "/shaders/examples/hello_sphere_rt/raygen.spv";
    const auto rayTracingMissShaderPath = KDGpu::assetPath() + "/shaders/examples/hello_sphere_rt/miss.spv";
    const auto rayTracingClosestShaderPath = KDGpu::assetPath() + "/shaders/examples/hello_sphere_rt/closest.spv";
    const auto rayTracingIntersectionShaderPath = KDGpu::assetPath() + "/shaders/examples/hello_sphere_rt/intersection.spv";

    auto rayTracingGenShader = m_device.createShaderModule(readShaderFile(rayTracingGenShaderPath));
    auto rayTracingMissShader = m_device.createShaderModule(readShaderFile(rayTracingMissShaderPath));
    auto rayTracingClosestShader = m_device.createShaderModule(readShaderFile(rayTracingClosestShaderPath));
    auto rayTracingIntersectionShader = m_device.createShaderModule(readShaderFile(rayTracingIntersectionShaderPath));

    // Create bind group layout consisting of an acceleration structure and an image to write out to
    const BindGroupLayoutOptions rtBindGroupLayoutOptions = {
        .bindings = {
                {
                        // Acceleration Structure
                        .binding = 0,
                        .count = 1,
                        .resourceType = ResourceBindingType::AccelerationStructure,
                        .shaderStages = ShaderStageFlags(ShaderStageFlagBits::RaygenBit),
                },
                {
                        // Output Image
                        .binding = 1,
                        .count = 1,
                        .resourceType = ResourceBindingType::StorageImage,
                        .shaderStages = ShaderStageFlagBits::RaygenBit | ShaderStageFlagBits::MissBit | ShaderStageFlagBits::ClosestHitBit,
                },
        },
    };
    const BindGroupLayoutOptions cameraBindGroupLayoutOptions = {
        .bindings = {
                {
                        // Camera UBO
                        .binding = 0,
                        .count = 1,
                        .resourceType = ResourceBindingType::UniformBuffer,
                        .shaderStages = ShaderStageFlags(ShaderStageFlagBits::RaygenBit),
                },
        },
    };
    const BindGroupLayoutOptions spheresBindGroupLayoutOptions = {
        .bindings = {
                {
                        // Sphere SSBO
                        .binding = 0,
                        .count = 1,
                        .resourceType = ResourceBindingType::StorageBuffer,
                        .shaderStages = ShaderStageFlagBits::IntersectionBit | ShaderStageFlagBits::ClosestHitBit,
                },
        },
    };

    m_rtBindGroupLayout = m_device.createBindGroupLayout(rtBindGroupLayoutOptions);
    m_cameraBindGroupLayout = m_device.createBindGroupLayout(cameraBindGroupLayoutOptions);
    m_sphereDataBindGroupLayout = m_device.createBindGroupLayout(spheresBindGroupLayoutOptions);

    // Create a pipeline layout (array of bind group layouts)
    const PipelineLayoutOptions pipelineLayoutOptions = {
        .label = "RT",
        .bindGroupLayouts = { m_rtBindGroupLayout, m_cameraBindGroupLayout, m_sphereDataBindGroupLayout }
    };
    m_pipelineLayout = m_device.createPipelineLayout(pipelineLayoutOptions);

    // Create a raytracing pipeline
    const RayTracingPipelineOptions pipelineOptions{
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
                        .shaderModule = rayTracingClosestShader.handle(),
                        .stage = ShaderStageFlagBits::ClosestHitBit,
                },
                ShaderStage{
                        .shaderModule = rayTracingIntersectionShader.handle(),
                        .stage = ShaderStageFlagBits::IntersectionBit,
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
                // Closest Hit
                RayTracingShaderGroupOptions{
                        .type = RayTracingShaderGroupType::ProceduralHit,
                        .closestHitShaderIndex = 2,
                        .intersectionShaderIndex = 3,
                },
        },
        .layout = m_pipelineLayout,
    };
    m_pipeline = m_device.createRayTracingPipeline(pipelineOptions);
}

void HelloSphereRt::createShaderBindingTable()
{
    // Create Shader Binding Table
    // This basically allows use to create a selection of ShaderGroups we want to use for a specific trace call
    // e.g which rayGen, which Miss, which Hit group we want to use
    // https://docs.vulkan.org/spec/latest/chapters/raytracing.html#shader-binding-table
    // https://www.willusher.io/graphics/2019/11/20/the-sbt-three-ways
    m_sbt = RayTracingShaderBindingTable(&m_device, RayTracingShaderBindingTableOptions{
                                                            .nbrMissShaders = 1,
                                                            .nbrHitShaders = 1,
                                                    });

    m_sbt.addRayGenShaderGroup(m_pipeline, 0);
    m_sbt.addMissShaderGroup(m_pipeline, 1);
    m_sbt.addHitShaderGroup(m_pipeline, 2);
}

void HelloSphereRt::createAccelerationStructures()
{
    const size_t SphereCount = 1024;

    struct SphereData {
        glm::vec4 positionAndRadius;
        glm::vec4 color;
    };
    static_assert(sizeof(SphereData) == 8 * sizeof(float));

    std::vector<SphereData> spheres(SphereCount);
    std::vector<VkAabbPositionsKHR> aabbs(SphereCount);

    std::random_device rd; // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()

    std::uniform_int_distribution<> posDistrib(0, 1024);
    std::uniform_int_distribution<> colorDistrib(0, 255);
    std::uniform_int_distribution<> radiusDistrib(0, 255);

    for (size_t i = 0; i < SphereCount; ++i) {
        SphereData &sphereData = spheres[i];
        for (size_t j = 0; j < 3; ++j) {
            sphereData.positionAndRadius[j] = (float(posDistrib(gen)) - 512.0f) / 512.0f * 100.0f;
            sphereData.color[j] = float(colorDistrib(gen)) / 255.0f;
        }
        sphereData.positionAndRadius[3] = std::fabs(float(radiusDistrib(gen)) / 255.0f) * 3.0f;

        const glm::vec3 minXYZ = glm::vec3(sphereData.positionAndRadius) - glm::vec3(sphereData.positionAndRadius[3]);
        const glm::vec3 maxXYZ = glm::vec3(sphereData.positionAndRadius) + glm::vec3(sphereData.positionAndRadius[3]);

        // SPDLOG_WARN("Position x: {}, y: {}, z: {}", sphereData.positionAndRadius[0], sphereData.positionAndRadius[1], sphereData.positionAndRadius[2]);

        aabbs[i] = VkAabbPositionsKHR{
            .minX = minXYZ.x,
            .minY = minXYZ.y,
            .minZ = minXYZ.z,
            .maxX = maxXYZ.x,
            .maxY = maxXYZ.y,
            .maxZ = maxXYZ.z,
        };
    };

    // Create a Buffer to hold the AABB of the spheres
    m_aabbBuffer = m_device.createBuffer(BufferOptions{
            .size = aabbs.size() * sizeof(VkAabbPositionsKHR),
            .usage = BufferUsageFlagBits::TransferSrcBit | BufferUsageFlagBits::TransferDstBit | BufferUsageFlagBits::AccelerationStructureBuildInputReadOnlyBit | BufferUsageFlagBits::ShaderDeviceAddressBit,
            .memoryUsage = MemoryUsage::CpuToGpu,
    });

    std::memcpy(m_aabbBuffer.map(), aabbs.data(), aabbs.size() * sizeof(VkAabbPositionsKHR));
    m_aabbBuffer.unmap();

    // Create SSBO to hold Sphere Information
    m_sphereDataSSBOBuffer = m_device.createBuffer(BufferOptions{
            .size = aabbs.size() * sizeof(VkAabbPositionsKHR),
            .usage = BufferUsageFlagBits::TransferDstBit | BufferUsageFlagBits::StorageBufferBit,
            .memoryUsage = MemoryUsage::CpuToGpu,
    });

    std::memcpy(m_sphereDataSSBOBuffer.map(), spheres.data(), spheres.size() * sizeof(SphereData));
    m_sphereDataSSBOBuffer.unmap();

    const AccelerationStructureGeometryAabbsData aabbGeometry{
        .data = m_aabbBuffer,
        .stride = sizeof(VkAabbPositionsKHR),
    };

    // Create Acceleration Structures (the BoundingVolumes we will ray trace against)

    // We will have SphereCount aabbGeometry
    m_bottomLevelAs = m_device.createAccelerationStructure(AccelerationStructureOptions{
            .label = "BottomLevelAS",
            .type = AccelerationStructureType::BottomLevel,
            .flags = AccelerationStructureFlagBits::PreferFastTrace,
            .geometryTypesAndCount = {
                    {
                            .geometry = aabbGeometry,
                            .maxPrimitiveCount = SphereCount,
                    },
            },
    });

    const AccelerationStructureGeometryInstancesData aabbGeometryInstance{
        .data = {
                AccelerationStructureGeometryInstance{
                        .flags = GeometryInstanceFlagBits::TriangleFacingCullDisable,
                        .accelerationStructure = m_bottomLevelAs,
                },
        },
    };

    // Add the instance information for our AABB
    m_topLevelAs = m_device.createAccelerationStructure(AccelerationStructureOptions{
            .label = "TopLevelAS",
            .type = AccelerationStructureType::TopLevel,
            .flags = AccelerationStructureFlagBits::PreferFastTrace,
            .geometryTypesAndCount = {
                    {
                            .geometry = aabbGeometryInstance,
                            .maxPrimitiveCount = 1,
                    },
            },
    });

    // Note: the geometries provided to create the AccelerationStructures were only used to compute
    // their size. Geometries will only be effectively linked to our AccelerationStructures when we build them below.

    // Build acceleration structures
    {
        auto commandRecorder = m_device.createCommandRecorder();

        // Bottom Level AS
        commandRecorder.beginDebugLabel(DebugLabelOptions{
                .label = "BottomLevel - AccelerationStructures",
                .color = { 0.0f, 1.0f, 0.0f, 1.0f },
        });

        commandRecorder.buildAccelerationStructures(BuildAccelerationStructureOptions{
                .buildGeometryInfos = {
                        {
                                .geometries = { aabbGeometry },
                                .destinationStructure = m_bottomLevelAs,
                                .buildRangeInfos = {
                                        {
                                                .primitiveCount = static_cast<uint32_t>(aabbs.size()),
                                                .primitiveOffset = 0,
                                                .firstVertex = 0,
                                                .transformOffset = 0,
                                        },
                                },
                        },
                },
        });

        // Pro Tip: If you don't want to spend days wondering why you have not hits...
        // => Make sure you wait for the bottomLevelAS to have been built prior to building the topLevelAS
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
        commandRecorder.endDebugLabel();

        // Top Level AS
        commandRecorder.beginDebugLabel(DebugLabelOptions{
                .label = "TopLevel - AccelerationStructures",
                .color = { 0.0f, 1.0f, 0.2f, 1.0f },
        });

        commandRecorder.buildAccelerationStructures(BuildAccelerationStructureOptions{
                .buildGeometryInfos = {
                        {
                                .geometries = { aabbGeometryInstance },
                                .destinationStructure = m_topLevelAs,
                                .buildRangeInfos = {
                                        {
                                                .primitiveCount = 1, // 1 BLAS
                                                .primitiveOffset = 0,
                                                .firstVertex = 0,
                                                .transformOffset = 0,
                                        },
                                },
                        },
                },
        });

        commandRecorder.endDebugLabel();

        CommandBuffer cmdBuffer = commandRecorder.finish();
        m_queue.submit(SubmitOptions{
                .commandBuffers = { cmdBuffer },
        });
        m_queue.waitUntilIdle();
    }
}

void HelloSphereRt::createBindGroups()
{

    // Create a bindGroup to hold the Acceleration Structure and Output Image
    {
        const BindGroupOptions bindGroupOptions = {
            .label = "RT Bind Group",
            .layout = m_rtBindGroupLayout,
            .resources = {
                    {
                            .binding = 0,
                            .resource = AccelerationStructureBinding{
                                    .accelerationStructure = m_topLevelAs,
                            },
                    },
                    // Too early to set output image
            }
        };
        // clang-format on
        m_rtBindGroup = m_device.createBindGroup(bindGroupOptions);
    }

    // Create camera BindGroup
    {
        const std::vector<uint8_t> rawCameraData = updateCameraData(1.0f, 1.0f, glm::vec3(0.0f, 0.0f, 100.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        m_cameraUBOBuffer = m_device.createBuffer(BufferOptions{
                                                          .size = rawCameraData.size(),
                                                          .usage = BufferUsageFlagBits::UniformBufferBit,
                                                          .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
                                                  },
                                                  rawCameraData.data());

        const BindGroupOptions bindGroupOptions = {
            .label = "Camera Bind Group",
            .layout = m_cameraBindGroupLayout,
            .resources = {
                    {
                            .binding = 0,
                            .resource = UniformBufferBinding{
                                    .buffer = m_cameraUBOBuffer,
                            },
                    },
            }
        };
        // clang-format on
        m_cameraBindGroup = m_device.createBindGroup(bindGroupOptions);
    }

    // Create spheres BindGroup
    {
        const BindGroupOptions bindGroupOptions = {
            .label = "Spheres Bind Group",
            .layout = m_sphereDataBindGroupLayout,
            .resources = {
                    {
                            .binding = 0,
                            .resource = StorageBufferBinding{
                                    .buffer = m_sphereDataSSBOBuffer,
                            },
                    },
            }
        };
        // clang-format on
        m_sphereDataBindGroup = m_device.createBindGroup(bindGroupOptions);
    }
}

void HelloSphereRt::initializeScene()
{
    createRayTracingPipeline();

    // Fill Shader Binding Table from Pipeline
    createShaderBindingTable();

    createAccelerationStructures();

    createBindGroups();

    m_swapchainImageLayouts = std::vector<KDGpu::TextureLayout>(m_swapchain.textures().size(), KDGpu::TextureLayout::Undefined);
    //![11]
}

void HelloSphereRt::cleanupScene()
{
    m_rtBindGroup = {};
    m_cameraBindGroup = {};
    m_sphereDataBindGroup = {};
    m_pipeline = {};
    m_pipelineLayout = {};
    m_rtBindGroupLayout = {};
    m_cameraBindGroupLayout = {};
    m_sphereDataBindGroupLayout = {};
    m_commandBuffer = {};
    m_topLevelAs = {};
    m_bottomLevelAs = {};
    m_aabbBuffer = {};
    m_cameraUBOBuffer = {};
    m_sphereDataSSBOBuffer = {};
    m_sbt = {};
}

//![1]
void HelloSphereRt::updateScene()
{
    static float angle = 0.0f;

    const float angleRad = glm::radians(angle);
    glm::vec3 pos = glm::vec3(std::cos(angleRad), 0.0f, std::sin(angleRad)) * std::max(0.1f, std::abs(std::sin(angleRad))) * 100.0f;

    angle += 0.1f;

    // Rotate Camera Around
    void *cameraData = m_cameraUBOBuffer.map();
    const std::vector<uint8_t> rawCameraData = updateCameraData(m_window->width(), std::max(m_window->height(), uint32_t(1)), pos, glm::vec3(0.0f));
    std::memcpy(cameraData, rawCameraData.data(), 2 * sizeof(glm::mat4));
    m_cameraUBOBuffer.unmap();
}
//![1]

void HelloSphereRt::resize()
{
    // Reset the layout entries
    m_swapchainImageLayouts = std::vector<KDGpu::TextureLayout>(m_swapchain.textures().size(), KDGpu::TextureLayout::Undefined);
}

//![2]
void HelloSphereRt::render()
{
    auto commandRecorder = m_device.createCommandRecorder();

    if (!m_swapchainImageLayouts.empty()) {
        // Transition Image to Presentation Layout
        const Handle<Texture_t> outputImage = m_swapchain.textures()[m_currentSwapchainImageIndex];

        // Transition Image to General Layout
        commandRecorder.textureMemoryBarrier(TextureMemoryBarrierOptions{
                .srcStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::TopOfPipeBit),
                .srcMask = KDGpu::AccessFlagBit::None,
                .dstStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::RayTracingShaderBit),
                .dstMask = KDGpu::AccessFlagBit::ShaderReadBit | KDGpu::AccessFlagBit::ShaderWriteBit,
                .oldLayout = m_swapchainImageLayouts[m_currentSwapchainImageIndex],
                .newLayout = KDGpu::TextureLayout::General,
                .texture = outputImage,
                .range = {
                        .aspectMask = KDGpu::TextureAspectFlagBits::ColorBit,
                        .levelCount = 1,
                },
        });

        // Update Image entry on BindGroup
        m_rtBindGroup.update(BindGroupEntry{
                .binding = 1,
                .resource = ImageBinding{
                        .textureView = m_swapchainViews[m_currentSwapchainImageIndex],
                },
        });

        commandRecorder.beginDebugLabel(DebugLabelOptions{
                .label = "RayTracing Pass",
                .color = { 1.0f, 0.0f, 0.0f, 1.0f },
        });

        auto rtPass = commandRecorder.beginRayTracingPass();
        rtPass.setPipeline(m_pipeline);
        rtPass.setBindGroup(0, m_rtBindGroup);
        rtPass.setBindGroup(1, m_cameraBindGroup);
        rtPass.setBindGroup(2, m_sphereDataBindGroup);

        // Issue RT Trace call using the SBT table we previously filled
        rtPass.traceRays(RayTracingCommand{
                .raygenShaderBindingTable = m_sbt.rayGenShaderRegion(),
                .missShaderBindingTable = m_sbt.missShaderRegion(),
                .hitShaderBindingTable = m_sbt.hitShaderRegion(),
                .extent = {
                        .width = m_swapchainExtent.width,
                        .height = m_swapchainExtent.height,
                        .depth = 1,
                },
        });

        rtPass.end();
        commandRecorder.endDebugLabel();

        // Transition Image to ColorAttachment Layout
        commandRecorder.textureMemoryBarrier(TextureMemoryBarrierOptions{
                .srcStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::RayTracingShaderBit),
                .srcMask = KDGpu::AccessFlagBit::ShaderReadBit | KDGpu::AccessFlagBit::ShaderWriteBit,
                .dstStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::TopOfPipeBit),
                .dstMask = KDGpu::AccessFlagBit::None,
                .oldLayout = KDGpu::TextureLayout::General,
                .newLayout = KDGpu::TextureLayout::ColorAttachmentOptimal,
                .texture = outputImage,
                .range = {
                        .aspectMask = KDGpu::TextureAspectFlagBits::ColorBit,
                        .levelCount = 1,
                },
        });

        commandRecorder.beginDebugLabel(DebugLabelOptions{
                .label = "Raster Pass",
                .color = { 0.0f, 0.0f, 1.0f, 1.0f },
        });

        // Create a GraphicsRenderPass to draw the imgui overlay
        auto opaquePass = commandRecorder.beginRenderPass(RenderPassCommandRecorderOptions{
                .colorAttachments = {
                        {
                                .view = m_swapchainViews[m_currentSwapchainImageIndex],
                                .loadOperation = AttachmentLoadOperation::Load,
                                .clearValue = { 0.0f, 0.0f, 0.0f, 0.0f },
                                .initialLayout = TextureLayout::ColorAttachmentOptimal,
                                .finalLayout = TextureLayout::PresentSrc,
                        },
                },
                .depthStencilAttachment = {
                        .view = m_depthTextureView,
                },
        });
        renderImGuiOverlay(&opaquePass);
        opaquePass.end();
        commandRecorder.endDebugLabel();

        // Update layout so that we know what layout we are in on the next frames
        m_swapchainImageLayouts[m_currentSwapchainImageIndex] = KDGpu::TextureLayout::PresentSrc;
    }

    m_commandBuffer = commandRecorder.finish();

    //![13]
    const SubmitOptions submitOptions = {
        .commandBuffers = { m_commandBuffer },
        .waitSemaphores = { m_presentCompleteSemaphores[m_inFlightIndex] },
        .signalSemaphores = { m_renderCompleteSemaphores[m_inFlightIndex] }
    };
    m_queue.submit(submitOptions);
    //![13]
}
//![2]

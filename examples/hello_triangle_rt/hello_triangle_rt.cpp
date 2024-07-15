/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "hello_triangle_rt.h"

#include <KDGpuExample/engine.h>
#include <KDGpuExample/kdgpuexample.h>

#include <KDGpu/acceleration_structure.h>
#include <KDGpu/acceleration_structure_options.h>
#include <KDGpu/bind_group_layout_options.h>
#include <KDGpu/bind_group_options.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/memory_barrier.h>
#include <KDGpu/graphics_pipeline_options.h>
#include <KDGpu/raytracing_pipeline.h>
#include <KDGpu/raytracing_pipeline_options.h>
#include <KDGpu/raytracing_pass_command_recorder.h>

#include <glm/glm.hpp>

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

HelloTriangleRt::HelloTriangleRt()
    : SimpleExampleEngineLayer()
{
    // Request our Swapchain Images to be usable as Storage Image
    m_swapchainUsageFlags = (TextureUsageFlagBits::ColorAttachmentBit | TextureUsageFlagBits::StorageBit);
}

void HelloTriangleRt::createRayTracingPipeline()
{
    // Create raytracing shaders
    const auto rayTracingGenShaderPath = KDGpu::assetPath() + "/shaders/examples/hello_triangle_rt/raygen.spv";
    const auto rayTracingMissShaderPath = KDGpu::assetPath() + "/shaders/examples/hello_triangle_rt/miss.spv";
    const auto rayTracingClosestShaderPath = KDGpu::assetPath() + "/shaders/examples/hello_triangle_rt/closest.spv";

    auto rayTracingGenShader = m_device.createShaderModule(readShaderFile(rayTracingGenShaderPath));
    auto rayTracingMissShader = m_device.createShaderModule(readShaderFile(rayTracingMissShaderPath));
    auto rayTracingClosestShader = m_device.createShaderModule(readShaderFile(rayTracingClosestShaderPath));

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

    m_rtBindGroupLayout = m_device.createBindGroupLayout(rtBindGroupLayoutOptions);

    // Create a pipeline layout (array of bind group layouts)
    const PipelineLayoutOptions pipelineLayoutOptions = {
        .label = "RT",
        .bindGroupLayouts = { m_rtBindGroupLayout }
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
                        .type = RayTracingShaderGroupType::TrianglesHit,
                        .closestHitShaderIndex = 2,
                },
        },
        .layout = m_pipelineLayout,
        .maxRecursionDepth = 1,
    };
    m_pipeline = m_device.createRayTracingPipeline(pipelineOptions);
}

void HelloTriangleRt::createShaderBindingTable()
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

    m_sbt.addRayGenShaderGroup(m_pipeline, 0); // So index 0 in our SBT for GenShaders references ShaderGroup 0 of the Pipeline
    m_sbt.addMissShaderGroup(m_pipeline, 1); // So index 0 in our SBT for MissShaders references ShaderGroup 1 of the Pipeline
    m_sbt.addHitShaderGroup(m_pipeline, 2); // So index 0 in our SBT for HitShaders references ShaderGroup 2 of the Pipeline
}

void HelloTriangleRt::createAccelerationStructures()
{
    struct Vertex {
        float x, y, z;
    };

    // Create a Buffer to hold our triangle vertices
    m_vertexBuffer = m_device.createBuffer(BufferOptions{
            .size = 6 * sizeof(Vertex),
            .usage = BufferUsageFlagBits::TransferSrcBit | BufferUsageFlagBits::TransferDstBit | BufferUsageFlagBits::AccelerationStructureBuildInputReadOnlyBit | BufferUsageFlagBits::ShaderDeviceAddressBit,
            .memoryUsage = MemoryUsage::CpuToGpu,
    });

    const std::vector<Vertex> vertices{
        { 0.0f, 1.0f, 0.0f },
        { -1.0f, -1.0f, 0.0f },
        { 1.0f, -1.0f, 0.0f },
    };
    std::memcpy(m_vertexBuffer.map(), vertices.data(), vertices.size() * sizeof(Vertex));
    m_vertexBuffer.unmap();

    const AccelerationStructureGeometryTrianglesData triangleDataGeometry{
        .vertexFormat = Format::R32G32B32_SFLOAT,
        .vertexData = m_vertexBuffer,
        .vertexStride = sizeof(Vertex),
        .maxVertex = 2, // This is an index not a count
    };

    // Create Acceleration Structures (the TriangleBasedBoundingVolume we will ray trace against)
    m_bottomLevelAs = m_device.createAccelerationStructure(AccelerationStructureOptions{
            .label = "BottomLevelAS",
            .type = AccelerationStructureType::BottomLevel,
            .flags = AccelerationStructureFlagBits::PreferFastTrace,
            .geometryTypesAndCount = {
                    {
                            .geometry = triangleDataGeometry,
                            .maxPrimitiveCount = 1, // We have a single triangles
                    },
            },
    });

    const AccelerationStructureGeometryInstancesData triGeometryInstance{
        .data = {
                AccelerationStructureGeometryInstance{
                        // clang-format off
                        // Apply a top level transform to scale our BottomLevel AS
                        .transform = {
                                0.5f, 0.0f, 0.0f, 0.0f,
                                0.0f, 0.5f, 0.0f, 0.0f,
                                0.0f, 0.0f, 0.5f, 0.0f,
                        },
                        // clang-format on
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
                            .geometry = triGeometryInstance,
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
        commandRecorder.buildAccelerationStructures(BuildAccelerationStructureOptions{
                .buildGeometryInfos = {
                        {
                                .geometries = { triangleDataGeometry },
                                .destinationStructure = m_bottomLevelAs,
                                .buildRangeInfos = {
                                        {
                                                .primitiveCount = 1, // A single triangle
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

        // Top Level AS
        commandRecorder.buildAccelerationStructures(BuildAccelerationStructureOptions{
                .buildGeometryInfos = {
                        {
                                .geometries = { triGeometryInstance },
                                .destinationStructure = m_topLevelAs,
                                .buildRangeInfos = {
                                        {
                                                .primitiveCount = 1,
                                                .primitiveOffset = 0,
                                                .firstVertex = 0,
                                                .transformOffset = 0,
                                        },
                                },
                        },
                },
        });

        CommandBuffer cmdBuffer = commandRecorder.finish();
        m_queue.submit(SubmitOptions{
                .commandBuffers = { cmdBuffer },
        });
        m_queue.waitUntilIdle();
    }
}

void HelloTriangleRt::createBindGroups()
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
}

void HelloTriangleRt::initializeScene()
{
    createRayTracingPipeline();

    // Fill Shader Binding Table from Pipeline
    createShaderBindingTable();

    createAccelerationStructures();

    createBindGroups();

    m_swapchainImageLayouts = std::vector<KDGpu::TextureLayout>(m_swapchain.textures().size(), KDGpu::TextureLayout::Undefined);
    //![11]
}

void HelloTriangleRt::cleanupScene()
{
    m_rtBindGroup = {};
    m_pipeline = {};
    m_pipelineLayout = {};
    m_rtBindGroupLayout = {};
    m_commandBuffer = {};
    m_topLevelAs = {};
    m_bottomLevelAs = {};
    m_vertexBuffer = {};
    m_sbt = {};
}

//![1]
void HelloTriangleRt::updateScene()
{
}
//![1]

void HelloTriangleRt::resize()
{
    // Reset the layout entries
    m_swapchainImageLayouts = std::vector<KDGpu::TextureLayout>(m_swapchain.textures().size(), KDGpu::TextureLayout::Undefined);
}

//![2]
void HelloTriangleRt::render()
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

        auto rtPass = commandRecorder.beginRayTracingPass();
        rtPass.setPipeline(m_pipeline);
        rtPass.setBindGroup(0, m_rtBindGroup);

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

        // Transition Image to PresentSrc Layout
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

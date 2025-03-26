/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "bindgroup_indexing.h"

#include <KDGpuExample/engine.h>
#include <KDGpuExample/kdgpuexample.h>

#include <KDGpu/bind_group_layout_options.h>
#include <KDGpu/bind_group_options.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/graphics_pipeline_options.h>
#include <KDGpu/texture_options.h>
#include <KDGpu/device.h>
#include <KDGpu/adapter.h>

#include <glm/gtx/transform.hpp>

#include <cmath>
#include <fstream>
#include <string>

namespace {
const size_t TransformsCount = 16;
}

BindGroupIndexing::BindGroupIndexing()
    : SimpleExampleEngineLayer()
{
}

void BindGroupIndexing::initializeScene()
{
    // Check that our device actually supports the Vulkan Descriptor Indexing features
    const AdapterFeatures &features = m_device.adapter()->features();
    if (!features.shaderUniformBufferArrayNonUniformIndexing ||
        !features.runtimeBindGroupArray) {
        SPDLOG_CRITICAL("Dynamic BindGroup Indexing is not supported, can't run this example");
        exit(0);
    }

    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
    };

    // Create a buffer to hold triangle vertex data
    {
        const float r = 0.8f;
        const std::array<Vertex, 3> vertexData = {
            Vertex{ // Bottom-left, red
                    .position = { r * cosf(7.0f * M_PI / 6.0f), -r * sinf(7.0f * M_PI / 6.0f), 0.0f },
                    .color = { 1.0f, 0.0f, 0.0f } },
            Vertex{ // Bottom-right, green
                    .position = { r * cosf(11.0f * M_PI / 6.0f), -r * sinf(11.0f * M_PI / 6.0f), 0.0f },
                    .color = { 0.0f, 1.0f, 0.0f } },
            Vertex{ // Top, blue
                    .position = { 0.0f, -r, 0.0f },
                    .color = { 0.0f, 0.0f, 1.0f } }
        };

        const DeviceSize dataByteSize = vertexData.size() * sizeof(Vertex);
        const BufferOptions bufferOptions = {
            .size = dataByteSize,
            .usage = BufferUsageFlagBits::VertexBufferBit | BufferUsageFlagBits::TransferDstBit,
            .memoryUsage = MemoryUsage::GpuOnly
        };
        m_buffer = m_device.createBuffer(bufferOptions);
        const BufferUploadOptions uploadOptions = {
            .destinationBuffer = m_buffer,
            .dstStages = PipelineStageFlagBit::VertexAttributeInputBit,
            .dstMask = AccessFlagBit::VertexAttributeReadBit,
            .data = vertexData.data(),
            .byteSize = dataByteSize
        };
        uploadBufferData(uploadOptions);
    }

    // Create a buffer to hold the geometry index data
    {
        const std::array<uint32_t, 3> indexData = { 0, 1, 2 };
        const DeviceSize dataByteSize = indexData.size() * sizeof(uint32_t);
        const BufferOptions bufferOptions = {
            .size = dataByteSize,
            .usage = BufferUsageFlagBits::IndexBufferBit | BufferUsageFlagBits::TransferDstBit,
            .memoryUsage = MemoryUsage::GpuOnly
        };
        m_indexBuffer = m_device.createBuffer(bufferOptions);
        const BufferUploadOptions uploadOptions = {
            .destinationBuffer = m_indexBuffer,
            .dstStages = PipelineStageFlagBit::IndexInputBit,
            .dstMask = AccessFlagBit::IndexReadBit,
            .data = indexData.data(),
            .byteSize = dataByteSize
        };
        uploadBufferData(uploadOptions);
    }

    //![1]
    // Create a set of TransformsCount UBOs, each holding a distinct rotation matrix
    {
        const BufferOptions bufferOptions = {
            .size = sizeof(glm::mat4),
            .usage = BufferUsageFlagBits::UniformBufferBit,
            .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
        };

        m_transformBuffers.reserve(TransformsCount);

        const float angleStep = 360.0f / float(TransformsCount);

        for (size_t i = 0; i < TransformsCount; ++i) {
            const glm::mat4 mat = glm::rotate(glm::mat4(1.0f), glm::radians(i * angleStep), glm::vec3(0.0f, 0.0f, 1.0f));

            Buffer buf = m_device.createBuffer(bufferOptions);
            auto bufferData = buf.map();
            std::memcpy(bufferData, &mat, sizeof(glm::mat4));
            buf.unmap();

            m_transformBuffers.emplace_back(std::move(buf));
        }
    }
    //![1]

    //![2]
    // Create a SSBO that will hold a frameCounter
    {
        m_frameCounterSSBO = m_device.createBuffer(BufferOptions{
                .size = sizeof(uint32_t),
                .usage = BufferUsageFlagBits::StorageBufferBit,
                .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
        });

        auto bufferData = m_frameCounterSSBO.map();
        std::memset(bufferData, 0, sizeof(uint32_t));
        m_frameCounterSSBO.unmap();
    }
    //![2]

    // Create a vertex shader and fragment shader
    auto vertexShaderPath = KDGpuExample::assetDir().file("shaders/examples/bindgroup_indexing/bindgroup_indexing.vert.spv");
    auto vertexShader = m_device.createShaderModule(KDGpuExample::readShaderFile(vertexShaderPath));

    auto fragmentShaderPath = KDGpuExample::assetDir().file("shaders/examples/bindgroup_indexing/bindgroup_indexing.frag.spv");
    auto fragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(fragmentShaderPath));

    //![3]
    // Create bind group layout consisting of a:
    // - a binding holding an array of at most TransformsCount UBOs
    // - a binding holding an SSBO
    const BindGroupLayoutOptions transformsBindGroupLayoutOptions = {
        .bindings = {
                {
                        .binding = 0,
                        .count = TransformsCount,
                        .resourceType = ResourceBindingType::UniformBuffer,
                        .shaderStages = ShaderStageFlagBits::VertexBit,
                        // As far as the shader is concerned, it has no idea how many UBOs are in the array
                        .flags = { ResourceBindingFlagBits::VariableBindGroupEntriesCountBit },
                } }
    };
    const BindGroupLayoutOptions ssboBindGroupLayoutOptions = {
        .bindings = {
                {
                        .binding = 0,
                        .resourceType = ResourceBindingType::StorageBuffer,
                        .shaderStages = ShaderStageFlagBits::VertexBit,
                },
        }
    };
    const BindGroupLayout transformsBindGroupLayout = m_device.createBindGroupLayout(transformsBindGroupLayoutOptions);
    const BindGroupLayout ssboBindGroupLayout = m_device.createBindGroupLayout(ssboBindGroupLayoutOptions);

    m_transformCountPushConstant = PushConstantRange{
        .offset = 0,
        .size = sizeof(uint32_t),
        .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit)
    };
    //![3]

    // Create a pipeline layout (array of bind group layouts)
    const PipelineLayoutOptions pipelineLayoutOptions = {
        .bindGroupLayouts = { transformsBindGroupLayout, ssboBindGroupLayout },
        .pushConstantRanges = { m_transformCountPushConstant }
    };
    m_pipelineLayout = m_device.createPipelineLayout(pipelineLayoutOptions);

    // Create a pipeline
    m_pipeline = m_device.createGraphicsPipeline(GraphicsPipelineOptions{
            .shaderStages = {
                    { .shaderModule = vertexShader, .stage = ShaderStageFlagBits::VertexBit },
                    { .shaderModule = fragmentShader, .stage = ShaderStageFlagBits::FragmentBit },
            },
            .layout = m_pipelineLayout,
            .vertex = {
                    .buffers = {
                            { .binding = 0, .stride = sizeof(Vertex) },
                    },
                    .attributes = {
                            { .location = 0, .binding = 0, .format = Format::R32G32B32_SFLOAT }, // Position
                            { .location = 1, .binding = 0, .format = Format::R32G32B32_SFLOAT, .offset = sizeof(glm::vec3) } // Color
                    },
            },
            .renderTargets = {
                    {
                            .format = m_swapchainFormat,
                    },
            },
            .depthStencil = {
                    .format = m_depthFormat,
                    .depthWritesEnabled = true,
                    .depthCompareOperation = CompareOperation::Less,
            },
    });

    {
        //![4]
        // Create a bindGroup to hold the variable length array UBOs
        {
            BindGroupOptions bindGroupOptions = {
                .layout = transformsBindGroupLayout,
                .maxVariableArrayLength = TransformsCount,
            };

            bindGroupOptions.resources.reserve(TransformsCount);

            // Array of UBOs
            for (size_t i = 0; i < TransformsCount; ++i) {
                bindGroupOptions.resources.emplace_back(BindGroupEntry{
                        .binding = 0,
                        .resource = UniformBufferBinding{ .buffer = m_transformBuffers[i] },
                        .arrayElement = static_cast<uint32_t>(i),
                });
            }

            m_transformsBindGroup = m_device.createBindGroup(bindGroupOptions);
        }
        //![4]

        //![5]
        // Create a bindGroup to hold the frameCounter SSBO
        {
            BindGroupOptions bindGroupOptions = {
                .layout = ssboBindGroupLayout,
                .resources = {
                        {
                                .binding = 0,
                                .resource = StorageBufferBinding{ .buffer = m_frameCounterSSBO },
                        },
                }
            };

            m_ssboBindGroup = m_device.createBindGroup(bindGroupOptions);
        }
        //![5]
    }

    // Most of the render pass is the same between frames. The only thing that changes, is which image
    // of the swapchain we wish to render to. So set up what we can here, and in the render loop we will
    // just update the color texture view.
    m_opaquePassOptions = {
        .colorAttachments = {
                { .view = {},
                  .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f },
                  .finalLayout = TextureLayout::PresentSrc } },
        .depthStencilAttachment = {
                .view = m_depthTextureView,
        },
    };
}

void BindGroupIndexing::cleanupScene()
{
    m_pipeline = {};
    m_pipelineLayout = {};
    m_buffer = {};
    m_indexBuffer = {};
    m_transformsBindGroup = {};
    m_ssboBindGroup = {};
    m_transformBuffers.clear();
    m_frameCounterSSBO = {};
    m_commandBuffer = {};
}

void BindGroupIndexing::updateScene()
{
}

void BindGroupIndexing::resize()
{
    // Swapchain might have been resized and texture views recreated. Ensure we update the PassOptions accordingly
    m_opaquePassOptions.depthStencilAttachment.view = m_depthTextureView;
}

void BindGroupIndexing::render()
{
    m_opaquePassOptions.colorAttachments[0].view = m_swapchainViews.at(m_currentSwapchainImageIndex);

    auto commandRecorder = m_device.createCommandRecorder();
    auto opaquePass = commandRecorder.beginRenderPass(m_opaquePassOptions);

    //![6]
    opaquePass.setPipeline(m_pipeline);
    opaquePass.setVertexBuffer(0, m_buffer);
    opaquePass.setIndexBuffer(m_indexBuffer);
    // Push Constant
    opaquePass.pushConstant(m_transformCountPushConstant, &TransformsCount);
    // Bind bindGroups
    opaquePass.setBindGroup(0, m_transformsBindGroup);
    opaquePass.setBindGroup(1, m_ssboBindGroup);
    const DrawIndexedCommand drawCmd = { .indexCount = 3 };
    opaquePass.drawIndexed(drawCmd);

    opaquePass.end();
    //![6]
    m_commandBuffer = commandRecorder.finish();

    const SubmitOptions submitOptions = {
        .commandBuffers = { m_commandBuffer },
        .waitSemaphores = { m_presentCompleteSemaphores[m_inFlightIndex] },
        .signalSemaphores = { m_renderCompleteSemaphores[m_inFlightIndex] }
    };
    m_queue.submit(submitOptions);
}

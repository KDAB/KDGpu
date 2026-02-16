/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "depth_bias.h"

#include <KDGpuExample/engine.h>
#include <KDGpuExample/kdgpuexample.h>

#include <KDGpu/bind_group_layout_options.h>
#include <KDGpu/bind_group_options.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/graphics_pipeline_options.h>

#include <glm/gtx/transform.hpp>

#include <cmath>
#include <fstream>
#include <string>

using namespace KDGpu;

void DepthBias::initializeScene()
{
    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
    };

    // Create a buffer to hold triangle vertex data
    {
        const float r = 0.8f;
        const std::array<Vertex, 6> vertexData = {
            Vertex{ // Bottom-left, red
                    .position = { r * cosf(7.0f * M_PI / 6.0f), -r * sinf(7.0f * M_PI / 6.0f), 0.0f },
                    .color = { 1.0f, 0.0f, 0.0f } },
            Vertex{ // Bottom-right, red
                    .position = { r * cosf(11.0f * M_PI / 6.0f), -r * sinf(11.0f * M_PI / 6.0f), 0.0f },
                    .color = { 1.0f, 0.0f, 0.0f } },
            Vertex{ // Top, red
                    .position = { 0.0f, -r, 0.0f },
                    .color = { 1.0f, 0.0f, 0.0f } },
            Vertex{ // Top-left, blue
                    .position = { -r * cosf(7.0f * M_PI / 6.0f), r * sinf(7.0f * M_PI / 6.0f), 0.0f },
                    .color = { 0.0f, 0.0f, 1.0f } },
            Vertex{ // Top-right, blue
                    .position = { -r * cosf(11.0f * M_PI / 6.0f), r * sinf(11.0f * M_PI / 6.0f), 0.0f },
                    .color = { 0.0f, 0.0f, 1.0f } },
            Vertex{ // Bottom, blue
                    .position = { 0.0f, r, 0.0f },
                    .color = { 0.0f, 0.0f, 1.0f } }
        };

        const DeviceSize dataByteSize = vertexData.size() * sizeof(Vertex);
        const BufferOptions bufferOptions = {
            .label = "Vertex Buffer",
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

    // Create a buffer to hold the transformation matrix
    {
        const BufferOptions bufferOptions = {
            .label = "Transformation Buffer",
            .size = sizeof(glm::mat4),
            .usage = BufferUsageFlagBits::UniformBufferBit,
            .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
        };
        m_transformBuffer = m_device.createBuffer(bufferOptions);

        // Upload identity matrix. Updated below in updateScene()
        m_transform = glm::mat4(1.0f);
        auto bufferData = m_transformBuffer.map();
        std::memcpy(bufferData, &m_transform, sizeof(glm::mat4));
        m_transformBuffer.unmap();
    }

    // Create a vertex shader and fragment shader
    auto vertexShaderPath = KDGpuExample::assetDir().file("shaders/examples/hello_triangle/hello_triangle.vert.spv");
    auto vertexShader = m_device.createShaderModule(KDGpuExample::readShaderFile(vertexShaderPath));

    auto fragmentShaderPath = KDGpuExample::assetDir().file("shaders/examples/hello_triangle/hello_triangle.frag.spv");
    auto fragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(fragmentShaderPath));

    // Create bind group layout consisting of a single binding holding a UBO
    const BindGroupLayoutOptions bindGroupLayoutOptions = {
        .label = "Transform Bind Group",
        .bindings = {
                {
                        .binding = 0,
                        .resourceType = ResourceBindingType::UniformBuffer,
                        .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit),
                },
        },
    };
    const BindGroupLayout bindGroupLayout = m_device.createBindGroupLayout(bindGroupLayoutOptions);

    // Create a pipeline layout (array of bind group layouts)
    const PipelineLayoutOptions pipelineLayoutOptions = {
        .label = "Triangle",
        .bindGroupLayouts = { bindGroupLayout }
    };
    m_pipelineLayout = m_device.createPipelineLayout(pipelineLayoutOptions);

    //![pipelines_creation]
    // Create pipelines for front and back triangles
    m_pipelineFront = m_device.createGraphicsPipeline(GraphicsPipelineOptions{
            .label = "TriangleFront",
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
            .renderTargets = { { .format = m_swapchainFormat } },
            .depthStencil = {
                    .format = m_depthFormat,
                    .depthWritesEnabled = true,
                    .depthCompareOperation = CompareOperation::LessOrEqual,
            },
    });
    m_pipelineBack = m_device.createGraphicsPipeline(GraphicsPipelineOptions{
            .label = "TriangleBack",
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
            .renderTargets = { { .format = m_swapchainFormat } },
            .depthStencil = {
                    .format = m_depthFormat,
                    .depthWritesEnabled = true,
                    .depthCompareOperation = CompareOperation::LessOrEqual,
            },
            .primitive = {
                    .depthBias{
                            .enabled = true,
                            .biasConstantFactor = 1.0f,
                    },
            },
    });
    //![pipelines_creation]

    // Create a bindGroup to hold the UBO with the transform
    const BindGroupOptions bindGroupOptions = {
        .label = "Transform Bind Group",
        .layout = bindGroupLayout,
        .resources = {
                {
                        .binding = 0,
                        .resource = UniformBufferBinding{ .buffer = m_transformBuffer },
                },
        },
    };
    m_transformBindGroup = m_device.createBindGroup(bindGroupOptions);
}

void DepthBias::cleanupScene()
{
    m_pipelineFront = {};
    m_pipelineBack = {};
    m_pipelineLayout = {};
    m_buffer = {};
    m_indexBuffer = {};
    m_transformBindGroup = {};
    m_transformBuffer = {};
    m_commandBuffer = {};
}

void DepthBias::updateScene()
{
    // Each frame we want to rotate the triangle a little
    static float angle = 0.0f;
    const float angularSpeed = 3.0f; // degrees per second
    const float dt = engine()->deltaTimeSeconds();
    angle += angularSpeed * dt;
    if (angle > 360.0f)
        angle -= 360.0f;

    m_transform = glm::mat4(1.0f);
    m_transform = glm::rotate(m_transform, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));

    auto bufferData = m_transformBuffer.map();
    std::memcpy(bufferData, &m_transform, sizeof(glm::mat4));
    m_transformBuffer.unmap();
}

void DepthBias::resize()
{
}

void DepthBias::render()
{
    auto commandRecorder = m_device.createCommandRecorder();

    auto opaquePass = commandRecorder.beginRenderPass(RenderPassCommandRecorderOptions{
            .colorAttachments = {
                    {
                            .view = m_swapchainViews.at(m_currentSwapchainImageIndex), // Not setting the swapchain texture view just yet
                            .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f },
                            .finalLayout = TextureLayout::PresentSrc,
                    },
            },
            .depthStencilAttachment = {
                    .view = m_depthTextureView,
            },
    });

    static int i = 0;
    const bool drawBlueInFront = (++i / 100) % 2 == 0;

    auto drawTriangle = [&](uint32_t vertexOffset, Handle<GraphicsPipeline_t> pipelineH) {
        opaquePass.setPipeline(pipelineH);
        opaquePass.setVertexBuffer(0, m_buffer);
        opaquePass.setBindGroup(0, m_transformBindGroup);
        opaquePass.draw(DrawCommand{ .vertexCount = 3, .firstVertex = vertexOffset });
    };

    // Draw Red Triangle
    drawTriangle(0, drawBlueInFront ? m_pipelineBack : m_pipelineFront);

    // Draw Blue Triangle
    drawTriangle(3, drawBlueInFront ? m_pipelineFront : m_pipelineBack);

    renderImGuiOverlay(&opaquePass);
    opaquePass.end();
    m_commandBuffer = commandRecorder.finish();

    const SubmitOptions submitOptions = {
        .commandBuffers = { m_commandBuffer },
        .waitSemaphores = { m_presentCompleteSemaphores[m_inFlightIndex] },
        .signalSemaphores = { m_renderCompleteSemaphores[m_currentSwapchainImageIndex] }
    };
    m_queue.submit(submitOptions);
}

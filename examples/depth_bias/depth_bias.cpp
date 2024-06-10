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

void DepthBias::initializeScene()
{
    //![3]
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
        //![4]
        m_buffer = m_device.createBuffer(bufferOptions);
        //![4]
        const BufferUploadOptions uploadOptions = {
            .destinationBuffer = m_buffer,
            .dstStages = PipelineStageFlagBit::VertexAttributeInputBit,
            .dstMask = AccessFlagBit::VertexAttributeReadBit,
            .data = vertexData.data(),
            .byteSize = dataByteSize
        };
        //![5]
        uploadBufferData(uploadOptions);
        //![5]
    }
    //![3]

    // Create a buffer to hold the transformation matrix
    //![7]
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
    //![7]

    // Create a vertex shader and fragment shader
    //![8]
    const auto vertexShaderPath = KDGpu::assetPath() + "/shaders/examples/hello_triangle/hello_triangle.vert.spv";
    auto vertexShader = m_device.createShaderModule(KDGpuExample::readShaderFile(vertexShaderPath));

    const auto fragmentShaderPath = KDGpu::assetPath() + "/shaders/examples/hello_triangle/hello_triangle.frag.spv";
    auto fragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(fragmentShaderPath));
    //![8]

    //![9]
    // Create bind group layout consisting of a single binding holding a UBO
    // clang-format off
    const BindGroupLayoutOptions bindGroupLayoutOptions = {
        .label = "Transform Bind Group",
        .bindings = {{
            .binding = 0,
            .resourceType = ResourceBindingType::UniformBuffer,
            .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit)
        }}
    };
    // clang-format on
    const BindGroupLayout bindGroupLayout = m_device.createBindGroupLayout(bindGroupLayoutOptions);

    // Create a pipeline layout (array of bind group layouts)
    const PipelineLayoutOptions pipelineLayoutOptions = {
        .label = "Triangle",
        .bindGroupLayouts = { bindGroupLayout }
    };
    m_pipelineLayout = m_device.createPipelineLayout(pipelineLayoutOptions);
    //![9]

    // Create a pipeline
    // clang-format off
    //![10]

    // clang-format on
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
    //![10]

    // Create a bindGroup to hold the UBO with the transform
    // clang-format off
    //![11]
    const BindGroupOptions bindGroupOptions = {
        .label = "Transform Bind Group",
        .layout = bindGroupLayout,
        .resources = {{
            .binding = 0,
            .resource = UniformBufferBinding{ .buffer = m_transformBuffer }
        }}
    };
    // clang-format on
    m_transformBindGroup = m_device.createBindGroup(bindGroupOptions);
    //![11]

    // Most of the render pass is the same between frames. The only thing that changes, is which image
    // of the swapchain we wish to render to. So set up what we can here, and in the render loop we will
    // just update the color texture view.
    // clang-format off
    //![12]
    m_opaquePassOptions = {
        .colorAttachments = {
            {
                .view = {}, // Not setting the swapchain texture view just yet
                .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f },
                .finalLayout = TextureLayout::PresentSrc
            }
        },
        .depthStencilAttachment = {
            .view = m_depthTextureView,
        }
    };
    //![12]
    // clang-format on
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

//![1]
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
//![1]

void DepthBias::resize()
{
    // Swapchain might have been resized and texture views recreated. Ensure we update the PassOptions accordingly
    m_opaquePassOptions.depthStencilAttachment.view = m_depthTextureView;
}

//![2]
void DepthBias::render()
{
    auto commandRecorder = m_device.createCommandRecorder();

    m_opaquePassOptions.colorAttachments[0].view = m_swapchainViews.at(m_currentSwapchainImageIndex);
    auto opaquePass = commandRecorder.beginRenderPass(m_opaquePassOptions);

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

/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "hello_triangle.h"

#include <KDGpuExample/engine.h>
#include <KDGpuExample/kdgpuexample.h>

#include <KDGpu/buffer_options.h>
#include <KDGpu/graphics_pipeline_options.h>

#include <fstream>
#include <string>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

void HelloTriangle::initializeScene()
{
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
        BufferOptions bufferOptions = {
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
        std::array<uint32_t, 3> indexData = { 0, 1, 2 };
        const DeviceSize dataByteSize = indexData.size() * sizeof(uint32_t);
        BufferOptions bufferOptions = {
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

    // Create a vertex shader and fragment shader
    const auto vertexShaderPath = KDGpu::assetPath() + "/shaders/examples/hello_triangle_overlap/hello_triangle.vert.spv";
    auto vertexShader = m_device.createShaderModule(KDGpuExample::readShaderFile(vertexShaderPath));

    const auto fragmentShaderPath = KDGpu::assetPath() + "/shaders/examples/hello_triangle_overlap/hello_triangle.frag.spv";
    auto fragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(fragmentShaderPath));

    // Create a pipeline layout (array of bind group layouts)
    const PipelineLayoutOptions pipelineLayoutOptions = {
        .pushConstantRanges = { m_transformPushConstantRange }
    };
    m_pipelineLayout = m_device.createPipelineLayout(pipelineLayoutOptions);

    // Create a pipeline
    // clang-format off
    const GraphicsPipelineOptions pipelineOptions = {
        .shaderStages = {
            { .shaderModule = vertexShader, .stage = ShaderStageFlagBits::VertexBit },
            { .shaderModule = fragmentShader, .stage = ShaderStageFlagBits::FragmentBit }
        },
        .layout = m_pipelineLayout,
        .vertex = {
            .buffers = {
                { .binding = 0, .stride = sizeof(Vertex) }
            },
            .attributes = {
                { .location = 0, .binding = 0, .format = Format::R32G32B32_SFLOAT }, // Position
                { .location = 1, .binding = 0, .format = Format::R32G32B32_SFLOAT, .offset = sizeof(glm::vec3) } // Color
            }
        },
        .renderTargets = {
            { .format = m_swapchainFormat }
        },
        .depthStencil = {
            .format = m_depthFormat,
            .depthWritesEnabled = true,
            .depthCompareOperation = CompareOperation::Less
        }
    };
    // clang-format on
    m_pipeline = m_device.createGraphicsPipeline(pipelineOptions);

    // Most of the render pass is the same between frames. The only thing that changes, is which image
    // of the swapchain we wish to render to. So set up what we can here, and in the render loop we will
    // just update the color texture view.
    // clang-format off
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
    // clang-format on
}

void HelloTriangle::cleanupScene()
{
    m_pipeline = {};
    m_pipelineLayout = {};
    m_indexBuffer = {};
    m_buffer = {};
    //![2]
    m_commandBuffers = {};
    //![2]
}

void HelloTriangle::updateScene()
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
}

void HelloTriangle::resize()
{
    // Swapchain might have been resized and texture views recreated. Ensure we update the PassOptions accordingly
    m_opaquePassOptions.depthStencilAttachment.view = m_depthTextureView;
}

void HelloTriangle::render()
{
    auto commandRecorder = m_device.createCommandRecorder();

    m_opaquePassOptions.colorAttachments[0].view = m_swapchainViews.at(m_currentSwapchainImageIndex);
    auto opaquePass = commandRecorder.beginRenderPass(m_opaquePassOptions);

    opaquePass.setPipeline(m_pipeline);
    opaquePass.setVertexBuffer(0, m_buffer);
    opaquePass.setIndexBuffer(m_indexBuffer);
    opaquePass.pushConstant(m_transformPushConstantRange, glm::value_ptr(m_transform));
    const DrawIndexedCommand drawCmd = { .indexCount = 3 };
    opaquePass.drawIndexed(drawCmd);
    //![1]
    renderImGuiOverlay(&opaquePass, m_inFlightIndex);
    opaquePass.end();
    m_commandBuffers[m_inFlightIndex] = commandRecorder.finish();

    const SubmitOptions submitOptions = {
        .commandBuffers = { m_commandBuffers[m_inFlightIndex] },
        .waitSemaphores = { m_presentCompleteSemaphores[m_inFlightIndex] }, // Wait for swapchain image acquisition
        .signalSemaphores = { m_renderCompleteSemaphores[m_inFlightIndex] },
        .signalFence = m_frameFences[m_inFlightIndex] // Signal Fence once submission and execution is complete
    };
    //![1]
    m_queue.submit(submitOptions);
}

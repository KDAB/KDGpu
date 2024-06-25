/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "buffer_reference.h"

#include <KDGpuExample/engine.h>
#include <KDGpuExample/kdgpuexample.h>

#include <KDGpu/bind_group_layout_options.h>
#include <KDGpu/bind_group_options.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/graphics_pipeline_options.h>
#include <KDGpu/texture_options.h>
#include <KDGpu/device.h>
#include <KDGpu/adapter.h>
#include <KDGpu/memory_barrier.h>

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

namespace {
const size_t TransformsCount = 16;
}

BufferReference::BufferReference()
    : SimpleExampleEngineLayer()
{
}

void BufferReference::initializeScene()
{
    // Check that our device actually supports the Vulkan Descriptor Indexing features
    const AdapterFeatures &features = m_device.adapter()->features();
    if (!features.bufferDeviceAddress) {
        SPDLOG_CRITICAL("Buffer Device Address is not supported, can't run this example");
        exit(0);
    }

    using Vertex = glm::vec3;

    // Create a buffer to hold triangle vertex data
    {
        const float r = 0.8f;
        const std::array<Vertex, 3> vertexData = {
            Vertex{
                    // Bottom Left
                    r * cosf(7.0f * M_PI / 6.0f),
                    -r * sinf(7.0f * M_PI / 6.0f),
                    0.0f,
            },
            Vertex{
                    // Bottom Right
                    r * cosf(11.0f * M_PI / 6.0f),
                    -r * sinf(11.0f * M_PI / 6.0f),
                    0.0f,
            },
            Vertex{
                    // Top
                    0.0f,
                    -r,
                    0.0f,
            }
        };

        const DeviceSize dataByteSize = vertexData.size() * sizeof(Vertex);
        const BufferOptions bufferOptions = {
            .size = dataByteSize,
            .usage = BufferUsageFlagBits::VertexBufferBit | BufferUsageFlagBits::TransferDstBit,
            .memoryUsage = MemoryUsage::GpuOnly
        };
        m_vertexBuffer = m_device.createBuffer(bufferOptions);
        const BufferUploadOptions uploadOptions = {
            .destinationBuffer = m_vertexBuffer,
            .dstStages = PipelineStageFlagBit::VertexAttributeInputBit,
            .dstMask = AccessFlagBit::VertexAttributeReadBit,
            .data = vertexData.data(),
            .byteSize = dataByteSize
        };
        uploadBufferData(uploadOptions);
    }

    // Create a buffer that can be referenced by its address that will hold vertex colors
    {
        const DeviceSize dataByteSize = 3 * sizeof(glm::vec4);
        const BufferOptions bufferOptions = {
            .size = dataByteSize,
            .usage = BufferUsageFlagBits::StorageBufferBit | BufferUsageFlagBits::ShaderDeviceAddressBit,
            .memoryUsage = MemoryUsage::CpuToGpu
        };
        m_vertexColorsBuffer = m_device.createBuffer(bufferOptions);
    }

    // Create a vertex shader and fragment shader
    const auto vertexShaderPath = KDGpu::assetPath() + "/shaders/examples/buffer_reference/buffer_reference.vert.spv";
    auto vertexShader = m_device.createShaderModule(KDGpuExample::readShaderFile(vertexShaderPath));

    const auto fragmentShaderPath = KDGpu::assetPath() + "/shaders/examples/buffer_reference/buffer_reference.frag.spv";
    auto fragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(fragmentShaderPath));

    // Create Push Constant that will hold the address of our vertexColorBuffer
    m_pushConstants = PushConstantRange{
        .offset = 0,
        .size = sizeof(BufferDeviceAddress),
        .shaderStages = ShaderStageFlagBits::VertexBit,
    };

    // Create a pipeline layout (array of bind group layouts)
    const PipelineLayoutOptions pipelineLayoutOptions = {
        .bindGroupLayouts = {},
        .pushConstantRanges = { m_pushConstants }
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

void BufferReference::cleanupScene()
{
    m_pipeline = {};
    m_pipelineLayout = {};
    m_vertexBuffer = {};
    m_vertexColorsBuffer = {};
    m_commandBuffer = {};
}

void BufferReference::updateScene()
{
    static float inc = 0.0f;

    const std::vector<glm::vec4> colors = {
        { std::abs(std::sin(inc)), std::abs(std::cos(inc)), std::abs(std::sin(inc) + std::cos(inc)), 1.0f },
        { std::abs(std::cos(inc)), std::abs(std::sin(inc) + std::cos(inc)), std::abs(std::cos(inc)), 1.0f },
        { std::abs(std::sin(inc) + std::cos(inc)), std::abs(std::sin(inc)), std::abs(std::cos(inc)), 1.0f },
    };

    // Update vertex color buffer
    glm::vec4 *vertexColors = reinterpret_cast<glm::vec4 *>(m_vertexColorsBuffer.map());
    std::memcpy(vertexColors, colors.data(), colors.size() * sizeof(glm::vec4));
    m_vertexColorsBuffer.unmap();

    inc += 0.01f;
}

void BufferReference::resize()
{
    // Swapchain might have been resized and texture views recreated. Ensure we update the PassOptions accordingly
    m_opaquePassOptions.depthStencilAttachment.view = m_depthTextureView;
}

void BufferReference::render()
{
    m_opaquePassOptions.colorAttachments[0].view = m_swapchainViews.at(m_currentSwapchainImageIndex);

    auto commandRecorder = m_device.createCommandRecorder();

    // Await any buffer transfers
    commandRecorder.bufferMemoryBarrier(BufferMemoryBarrierOptions{
            .srcStages = PipelineStageFlagBit::TransferBit,
            .srcMask = AccessFlagBit::TransferWriteBit,
            .dstStages = PipelineStageFlagBit::VertexShaderBit,
            .dstMask = AccessFlagBit::ShaderReadBit,
            .buffer = m_vertexColorsBuffer,
    });

    auto opaquePass = commandRecorder.beginRenderPass(m_opaquePassOptions);
    opaquePass.setPipeline(m_pipeline);
    opaquePass.setVertexBuffer(0, m_vertexBuffer);
    // Push Constant
    const BufferDeviceAddress vertexColorBufAddress = m_vertexColorsBuffer.bufferDeviceAddress();
    opaquePass.pushConstant(m_pushConstants, &vertexColorBufAddress);
    // Draw
    opaquePass.draw(DrawCommand{ .vertexCount = 3 });
    opaquePass.end();
    m_commandBuffer = commandRecorder.finish();

    const SubmitOptions submitOptions = {
        .commandBuffers = { m_commandBuffer },
        .waitSemaphores = { m_presentCompleteSemaphores[m_inFlightIndex] },
        .signalSemaphores = { m_renderCompleteSemaphores[m_inFlightIndex] }
    };
    m_queue.submit(submitOptions);
}

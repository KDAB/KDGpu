/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "multiview_stereo.h"

#include <KDGpuExample/engine.h>
#include <KDGpuExample/kdgpuexample.h>

#include <KDGpu/buffer_options.h>
#include <KDGpu/texture_options.h>
#include <KDGpu/graphics_pipeline_options.h>
#include <KDGpu/swapchain_options.h>

#include <fstream>
#include <string>

#include <glm/glm.hpp>

using namespace KDGpu;

void MultiViewStereo::initializeScene()
{
    //![1]

    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
    };

    // Create a buffer to hold triangle vertex data
    {
        m_vertexBuffer = m_device.createBuffer(BufferOptions{
                .size = 3 * sizeof(Vertex), // 3 vertices * 2 attributes * 3 float components
                .usage = BufferUsageFlagBits::VertexBufferBit,
                .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
        });

        const float r = 0.8f;
        std::array<Vertex, 3> vertexData;
        vertexData[0] = { { r * std::cos(7.0f * M_PI / 6.0f), -r * std::sin(7.0f * M_PI / 6.0f), 0.0f }, { 1.0f, 0.0, 0.0f } }; // Bottom-left, red
        vertexData[1] = { { r * std::cos(11.0f * M_PI / 6.0f), -r * std::sin(11.0f * M_PI / 6.0f), 0.0f }, { 0.0f, 1.0, 0.0f } }; // Bottom-right, green
        vertexData[2] = { { 0.0f, -r, 0.0f }, { 0.0f, 0.0, 1.0f } }; // Top, blue

        auto bufferData = m_vertexBuffer.map();
        std::memcpy(bufferData, vertexData.data(), vertexData.size() * sizeof(Vertex));
        m_vertexBuffer.unmap();
    }
    //![1]

    //![2]
    // Create a vertex shader and fragment shader (spir-v only for now)
    auto vertexShaderPath = KDGpuExample::assetDir().file("shaders/examples/multiview/rotating_triangle.vert.spv");
    auto vertexShader = m_device.createShaderModule(KDGpuExample::readShaderFile(vertexShaderPath));

    auto fragmentShaderPath = KDGpuExample::assetDir().file("shaders/examples/multiview/rotating_triangle.frag.spv");
    auto fragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(fragmentShaderPath));
    //![2]

    //![3]
    // Create a pipeline layout (array of bind group layouts)
    m_pipelineLayout = m_device.createPipelineLayout(PipelineLayoutOptions{
            .pushConstantRanges = { m_pushConstantRange },
    });
    //![3]

    //![4]
    m_pipeline = m_device.createGraphicsPipeline(GraphicsPipelineOptions{
            .shaderStages = {
                    { .shaderModule = vertexShader, .stage = ShaderStageFlagBits::VertexBit },
                    { .shaderModule = fragmentShader, .stage = ShaderStageFlagBits::FragmentBit } },
            .layout = m_pipelineLayout,
            .vertex = {
                    .buffers = {
                            { .binding = 0, .stride = sizeof(Vertex) },
                    },
                    .attributes = {
                            { .location = 0, .binding = 0, .format = Format::R32G32B32_SFLOAT }, // Position
                            { .location = 1, .binding = 0, .format = Format::R32G32B32_SFLOAT, .offset = sizeof(glm::vec3) }, // Color
                    },
            },
            .renderTargets = {
                    { .format = m_swapchainFormat },
            },
            .depthStencil = {
                    .format = m_depthFormat,
                    .depthWritesEnabled = true,
                    .depthCompareOperation = CompareOperation::Less,
            },
            .viewCount = 2, // We want to process and render 2 views at once
    });
    //![4]

    //![5]
    m_opaquePassOptions = {
        .colorAttachments = {
                { .view = {}, // Not setting the swapchain texture view just yet
                  .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f },
                  .finalLayout = TextureLayout::PresentSrc },
        },
        .depthStencilAttachment = { .view = m_depthTextureView },
        .viewCount = 2, // Enables multiview rendering
    };
    //![5]
}

void MultiViewStereo::cleanupScene()
{
    m_pipeline = {};
    m_pipelineLayout = {};
    m_vertexBuffer = {};

    m_commandBuffer = {};
}

//![6]
void MultiViewStereo::recreateSwapChain()
{
    const AdapterSwapchainProperties swapchainProperties = m_device.adapter()->swapchainProperties(m_surface);

    if (swapchainProperties.capabilities.maxImageArrayLayers < 2) {
        SPDLOG_CRITICAL("This setup does not support Stereo SwapChains");
    }

    // Create a swapchain of images that we will render to.
    SwapchainOptions swapchainOptions = {
        .surface = m_surface,
        .format = m_swapchainFormat,
        .minImageCount = getSuitableImageCount(swapchainProperties.capabilities),
        .imageExtent = { .width = m_window->width(), .height = m_window->height() },
        .imageLayers = 2,
        .presentMode = PresentMode::FifoRelaxed, // NVidia doesn't support MailBox with Stereo
        .oldSwapchain = m_swapchain,
    };

    // Create swapchain and destroy previous one implicitly
    m_swapchain = m_device.createSwapchain(swapchainOptions);

    const auto &swapchainTextures = m_swapchain.textures();
    const auto swapchainTextureCount = swapchainTextures.size();

    m_swapchainViews.clear();
    m_swapchainViews.reserve(swapchainTextureCount);
    for (uint32_t i = 0; i < swapchainTextureCount; ++i) {
        auto view = swapchainTextures[i].createView({
                .viewType = ViewType::ViewType2DArray,
                .format = swapchainOptions.format,
                .range = TextureSubresourceRange{
                        .aspectMask = TextureAspectFlagBits::ColorBit,
                        .baseArrayLayer = 0,
                        .layerCount = 2,
                },
        });
        m_swapchainViews.push_back(std::move(view));
    }

    // Create a depth texture to use for depth-correct rendering
    TextureOptions depthTextureOptions = {
        .type = TextureType::TextureType2D,
        .format = m_depthFormat,
        .extent = { m_window->width(), m_window->height(), 1 },
        .mipLevels = 1,
        .arrayLayers = 2,
        .samples = m_samples.get(),
        .usage = TextureUsageFlagBits::DepthStencilAttachmentBit | m_depthTextureUsageFlags,
        .memoryUsage = MemoryUsage::GpuOnly
    };
    m_depthTexture = m_device.createTexture(depthTextureOptions);
    m_depthTextureView = m_depthTexture.createView({
            .viewType = ViewType::ViewType2DArray,
            .range = TextureSubresourceRange{
                    .aspectMask = TextureAspectFlagBits::DepthBit,
                    .baseArrayLayer = 0,
                    .layerCount = 2,
            },
    });

    m_capabilitiesString = surfaceCapabilitiesToString(m_device.adapter()->swapchainProperties(m_surface).capabilities);
}
//![6]

void MultiViewStereo::updateScene()
{
    // Nothing to do for this simple, static, non-interactive example
}

void MultiViewStereo::resize()
{
    // Swapchain might have been resized and texture views recreated. Ensure we update the PassOptions accordingly
    m_opaquePassOptions.depthStencilAttachment.view = m_depthTextureView;
}

void MultiViewStereo::render()
{
    static float rotationAngleDeg = 0.0f;

    rotationAngleDeg += 0.1f;
    const float rotationAngleRad = glm::radians(rotationAngleDeg);

    // Create a command encoder/recorder
    auto commandRecorder = m_device.createCommandRecorder();

    //![7]
    // MultiViewStereo OpaquePass
    m_opaquePassOptions.colorAttachments[0].view = m_swapchainViews.at(m_currentSwapchainImageIndex);

    auto opaquePass = commandRecorder.beginRenderPass(m_opaquePassOptions);
    opaquePass.setPipeline(m_pipeline);
    opaquePass.setVertexBuffer(0, m_vertexBuffer);
    opaquePass.pushConstant(m_pushConstantRange, &rotationAngleRad);
    opaquePass.draw(DrawCommand{ .vertexCount = 3 });
    opaquePass.end();
    //![7]

    // End recording
    m_commandBuffer = commandRecorder.finish();

    // Submit command buffer to queue
    const SubmitOptions submitOptions = {
        .commandBuffers = { m_commandBuffer },
        .waitSemaphores = { m_presentCompleteSemaphores[m_inFlightIndex] },
        .signalSemaphores = { m_renderCompleteSemaphores[m_currentSwapchainImageIndex] }
    };
    m_queue.submit(submitOptions);
}

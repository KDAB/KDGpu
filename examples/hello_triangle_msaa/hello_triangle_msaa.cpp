/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "hello_triangle_msaa.h"
#include <imgui.h>

#include <KDGpuExample/engine.h>
#include <KDGpuExample/kdgpuexample.h>

#include <KDGpu/bind_group_layout_options.h>
#include <KDGpu/bind_group_options.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/graphics_pipeline_options.h>
#include <KDGpu/texture_options.h>

#include <glm/gtx/transform.hpp>

#include <cmath>
#include <fstream>
#include <string>

HelloTriangleMSAA::HelloTriangleMSAA()
    : SimpleExampleEngineLayer()
{
}

void HelloTriangleMSAA::initializeScene()
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

    // Create a buffer to hold the transformation matrix
    {
        const BufferOptions bufferOptions = {
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

    registerImGuiOverlayDrawFunction([this](ImGuiContext *ctx) {
        drawMsaaSettings(ctx);
    });

    // Create a vertex shader and fragment shader
    auto vertexShaderPath = KDGpuExample::assetDir().file("shaders/examples/hello_triangle_msaa/hello_triangle.vert.spv");
    auto vertexShader = m_device.createShaderModule(KDGpuExample::readShaderFile(vertexShaderPath));

    auto fragmentShaderPath = KDGpuExample::assetDir().file("shaders/examples/hello_triangle_msaa/hello_triangle.frag.spv");
    auto fragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(fragmentShaderPath));

    // Create bind group layout consisting of a single binding holding a UBO
    // clang-format off
    const BindGroupLayoutOptions bindGroupLayoutOptions = {
        .bindings = {{
            .binding = 0,
            .resourceType = ResourceBindingType::UniformBuffer,
            .shaderStages = ShaderStageFlagBits::VertexBit
        }}
    };
    // clang-format on
    const BindGroupLayout bindGroupLayout = m_device.createBindGroupLayout(bindGroupLayoutOptions);

    // Create a pipeline layout (array of bind group layouts)
    const PipelineLayoutOptions pipelineLayoutOptions = {
        .bindGroupLayouts = { bindGroupLayout }
    };
    m_pipelineLayout = m_device.createPipelineLayout(pipelineLayoutOptions);

    // Create a pipeline
    // clang-format off
    auto mkPipelineOptions = [&](SampleCountFlagBits samples) -> GraphicsPipelineOptions {
        return {
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
        },
        //![3]
        .multisample = {
            .samples = samples
        }
        //![3]
        };
    };
    // clang-format on

    // create pipelines for all supported sample counts. m_supportedSampleCounts
    // is populated by KDGpu::ExampleEngineLayer.
    for (auto sampleCount : m_supportedSampleCounts) {
        m_pipelines.push_back(m_device.createGraphicsPipeline(mkPipelineOptions(sampleCount)));
    }

    // Create a bindGroup to hold the UBO with the transform
    // clang-format off
    const BindGroupOptions bindGroupOptions = {
        .layout = bindGroupLayout,
        .resources = {{
            .binding = 0,
            .resource = UniformBufferBinding{ .buffer = m_transformBuffer }
        }}
    };
    // clang-format on
    m_transformBindGroup = m_device.createBindGroup(bindGroupOptions);

    // initialize pipeline, UI variable, and samples to all be the maximum supported MSAA level
    m_samples = m_supportedSampleCounts.back();
    m_requestedSampleCountIndex = m_supportedSampleCounts.size() - 1;
    m_currentPipelineIndex = m_requestedSampleCountIndex;

    // Most of the render pass is the same between frames. The only thing that changes, is which image
    // of the swapchain we wish to render to. So set up what we can here, and in the render loop we will
    // just update the color texture view.
    // clang-format off
    //![2]
    m_commandRecorderOptions = {
        .colorAttachments = {
            {
                .view = m_msaaTextureView, // The multisampled view which will change on resize.
                .resolveView = {}, // Not setting the swapchain texture view just yet. That's handled at render.
                .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f },
                .finalLayout = TextureLayout::PresentSrc
            }
        },
        .depthStencilAttachment = {
            .view = m_depthTextureView,
        },
        // configure for multisampling
        .samples = m_samples.get()
    };
    //![2]
    // clang-format on

    // Create a multisample texture into which we will render. The pipeline will then resolve the
    // multi-sampled texture into the current swapchain image.
    createRenderTarget();
}

void HelloTriangleMSAA::cleanupScene()
{
    m_pipelineLayout = {};
    m_msaaTextureView = {};
    m_msaaTexture = {};
    m_buffer = {};
    m_indexBuffer = {};
    m_transformBindGroup = {};
    m_transformBuffer = {};
    m_commandBuffer = {};
    m_pipelines.clear();
}

void HelloTriangleMSAA::updateScene()
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

    auto requested = m_supportedSampleCounts[m_requestedSampleCountIndex];
    if (requested != m_samples.get())
        setMsaaSampleCount(requested);
}

void HelloTriangleMSAA::resize()
{
    // Recreate the msaa render target texture
    createRenderTarget();
}
//![4]
void HelloTriangleMSAA::createRenderTarget()
{
    // Reset depthTextureView as depthStencilAttachment view as it might
    // have been recreated following a resize
    m_commandRecorderOptions.depthStencilAttachment.view = m_depthTextureView;

    const TextureOptions options = {
        .type = TextureType::TextureType2D,
        .format = m_swapchainFormat,
        .extent = { .width = m_swapchainExtent.width, .height = m_swapchainExtent.height, .depth = 1 },
        .mipLevels = 1,
        .samples = m_samples.get(),
        .usage = TextureUsageFlagBits::ColorAttachmentBit,
        .memoryUsage = MemoryUsage::GpuOnly,
        .initialLayout = TextureLayout::Undefined
    };
    m_msaaTexture = m_device.createTexture(options);
    m_msaaTextureView = m_msaaTexture.createView();

    if (isMsaaEnabled())
        m_commandRecorderOptions.colorAttachments[0].view = m_msaaTextureView;
}

bool HelloTriangleMSAA::isMsaaEnabled() const
{
    return m_samples.get() != SampleCountFlagBits::Samples1Bit;
}

void HelloTriangleMSAA::setMsaaSampleCount(SampleCountFlagBits samples)
{
    if (samples == m_samples.get())
        return;

    // get new pipeline
    for (size_t i = 0; i < m_supportedSampleCounts.size(); ++i) {
        if (m_supportedSampleCounts[i] == samples) {
            m_currentPipelineIndex = i;
            break;
        }
    }

    // the ExampleEngineLayer will recreate the depth view when we do this
    m_samples = samples;

    // we must also refresh the view(s) we handle, and reattach them
    createRenderTarget();

    // update the samples option that will configure the render pass
    m_commandRecorderOptions.samples = samples;
}

void HelloTriangleMSAA::drawMsaaSettings(ImGuiContext *ctx)
{
    constexpr ImVec2 winOffset(200, 150);
    constexpr ImVec2 buttonSize(120, 40);
    constexpr size_t maxMessageLen = 40;

    ImGui::SetCurrentContext(ctx);
    ImGui::SetNextWindowPos(ImVec2((float)m_window->width() - winOffset.x, winOffset.y));
    ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::Begin(
            "Controls",
            nullptr,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);

    auto getButtonLabel = [](SampleCountFlagBits samples) -> const char * {
        switch (samples) {
        case SampleCountFlagBits::Samples1Bit:
            return "No MSAA";
        case SampleCountFlagBits::Samples2Bit:
            return "2x MSAA";
        case SampleCountFlagBits::Samples4Bit:
            return "4x MSAA";
        case SampleCountFlagBits::Samples8Bit:
            return "8x MSAA";
        case SampleCountFlagBits::Samples16Bit:
            return "16x MSAA";
        case SampleCountFlagBits::Samples32Bit:
            return "32x MSAA";
        case SampleCountFlagBits::Samples64Bit:
            return "64x MSAA";
        default:
            return "Unknown";
        }
    };

    int selectedIndex = m_requestedSampleCountIndex;
    for (int i = 0; i < m_supportedSampleCounts.size(); ++i) {
        ImGui::RadioButton(getButtonLabel(m_supportedSampleCounts[i]), &selectedIndex, i);
    }

    // so we can deal with it in updateScene
    m_requestedSampleCountIndex = selectedIndex;

    ImGui::End();
}

//![4]
void HelloTriangleMSAA::render()
{
    if (isMsaaEnabled()) {
        //![1]
        // When using MSAA, we update the resolveView instead of the view
        m_commandRecorderOptions.colorAttachments[0].resolveView = m_swapchainViews.at(m_currentSwapchainImageIndex);
        //![1]
    } else {
        m_commandRecorderOptions.colorAttachments[0].view = m_swapchainViews.at(m_currentSwapchainImageIndex);
    }

    auto commandRecorder = m_device.createCommandRecorder();
    auto opaquePass = commandRecorder.beginRenderPass(m_commandRecorderOptions);

    opaquePass.setPipeline(m_pipelines[m_currentPipelineIndex]);
    opaquePass.setVertexBuffer(0, m_buffer);
    opaquePass.setIndexBuffer(m_indexBuffer);
    opaquePass.setBindGroup(0, m_transformBindGroup);
    const DrawIndexedCommand drawCmd = { .indexCount = 3 };
    opaquePass.drawIndexed(drawCmd);
    renderImGuiOverlay(&opaquePass);
    opaquePass.end();
    m_commandBuffer = commandRecorder.finish();

    const SubmitOptions submitOptions = {
        .commandBuffers = { m_commandBuffer },
        .waitSemaphores = { m_presentCompleteSemaphores[m_inFlightIndex] },
        .signalSemaphores = { m_renderCompleteSemaphores[m_inFlightIndex] }
    };
    m_queue.submit(submitOptions);
}

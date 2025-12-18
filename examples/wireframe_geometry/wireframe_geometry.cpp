/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "wireframe_geometry.h"

#include <KDGpuExample/engine.h>
#include <KDGpuExample/kdgpuexample.h>
#include <KDGpuExample/view_projection.h>

#include <KDGpu/bind_group_layout_options.h>
#include <KDGpu/bind_group_options.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/graphics_pipeline_options.h>
#include <KDGpu/texture_options.h>

#include <imgui.h>

#include <glm/gtx/transform.hpp>

#include <string>

using namespace KDGpu;

namespace {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    float excludeEdge; // Set to 1.0 to exclude edge opposite this vertex from the wireframe, 0.0 to include
};
static_assert(sizeof(Vertex) == 7 * sizeof(float));

std::vector<Vertex> initializeCubeMesh()
{
    // clang-format off
    const glm::vec3 A(1.0f, 1.0f, 1.0f);                      //       D ---------- C
    const glm::vec3 B(-1.0f, 1.0f, 1.0f);                     //      /|           /|
    const glm::vec3 C(1.0f, 1.0f, -1.0f);                     //     B ---------- A |
    const glm::vec3 D(-1.0f, 1.0f, -1.0f);                    //     | |          | |
    const glm::vec3 E(1.0f, -1.0f, 1.0f);                     //     | H ---------| G
    const glm::vec3 F(-1.0f, -1.0f, 1.0f);                    //     |/           |/
    const glm::vec3 G(1.0f, -1.0f, -1.0f);                    //     F ---------- E
    const glm::vec3 H(-1.0f, -1.0f, -1.0f);                   //

    const glm::vec3 nTop(0.0f, 1.0f, 0.0f);
    const glm::vec3 nBottom(0.0f, -1.0f, 0.0f);
    const glm::vec3 nFront(0.0f, 0.0f, -1.0f);
    const glm::vec3 nBack(0.0f, 0.0f, 1.0f);
    const glm::vec3 nLeft(1.0f, 0.0f, 0.0f);
    const glm::vec3 nRight(-1.0f, 0.0f, 0.0f);

    const float includeEdge = 0.0f;
    const float excludeEdge = 1.0f;

    return {
        // Top
        {A, nTop, includeEdge}, {C, nTop, excludeEdge}, {D, nTop, includeEdge},
        {D, nTop, includeEdge}, {B, nTop, excludeEdge}, {A, nTop, includeEdge},
        // Front
        {B, nFront, includeEdge}, {F, nFront, excludeEdge}, {E, nFront, includeEdge},
        {E, nFront, includeEdge}, {A, nFront, excludeEdge}, {B, nFront, includeEdge},
        // Back
        {G, nBack, includeEdge}, {H, nBack, excludeEdge}, {D, nBack, includeEdge},
        {D, nBack, includeEdge}, {C, nBack, excludeEdge}, {G, nBack, includeEdge},
        // Bottom
        {E, nBottom, includeEdge}, {F, nBottom, excludeEdge}, {H, nBottom, includeEdge},
        {H, nBottom, includeEdge}, {G, nBottom, excludeEdge}, {E, nBottom, includeEdge},
        // Left
        {F, nLeft, includeEdge}, {B, nLeft, excludeEdge}, {D, nLeft, includeEdge},
        {D, nLeft, includeEdge}, {H, nLeft, excludeEdge}, {F, nLeft, includeEdge},
        // Right
        {A, nRight, includeEdge}, {E, nRight, excludeEdge}, {G, nRight, includeEdge},
        {G, nRight, includeEdge}, {C, nRight, excludeEdge}, {A, nRight, includeEdge},
    };
    // clang-format on
}

} // namespace

void WireframeGeometry::initializeScene()
{
    // Select a reasonable sample count for our device. 8 samples or as good as we can get
    m_samples = SampleCountFlagBits::Samples1Bit;

    const std::array<SampleCountFlagBits, 3> suitableSamples{
        SampleCountFlagBits::Samples8Bit,
        SampleCountFlagBits::Samples4Bit,
        SampleCountFlagBits::Samples2Bit,
    };

    for (auto suitableSample : suitableSamples) {
        if (std::find(m_supportedSampleCounts.begin(), m_supportedSampleCounts.end(), suitableSample) != m_supportedSampleCounts.end()) {
            m_samples = suitableSample;
            break;
        }
    }

    // Create a buffer to hold triangle vertex data for the cube
    {
        const std::vector<Vertex> vertexData = initializeCubeMesh();
        const DeviceSize dataByteSize = vertexData.size() * sizeof(Vertex);
        const BufferOptions bufferOptions = {
            .label = "Vertex Buffer",
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

    // Create a buffer to hold the camera UBO
    {
        const BufferOptions bufferOptions = {
            .label = "Camera Buffer",
            .size = sizeof(CameraData),
            .usage = BufferUsageFlagBits::UniformBufferBit,
            .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
        };
        m_cameraBuffer = m_device.createBuffer(bufferOptions);
        m_cameraBufferData = m_cameraBuffer.map();

        // Set up initial camera data
        m_cameraData.view = glm::lookAt(m_cameraPosition,
                                        glm::vec3(0.0f, 0.0f, 0.0f),
                                        glm::vec3(0.0f, 1.0f, 0.0f));
        m_cameraData.projection = KDGpuExample::perspective({ .verticalFieldOfView = 60.0f,
                                                              .aspectRatio = float(m_swapchainExtent.width) / float(m_swapchainExtent.height),
                                                              .nearPlane = 0.1f,
                                                              .farPlane = 100.0f });

        std::memcpy(m_cameraBufferData, &m_cameraData, sizeof(CameraData));
    }

    // Create a buffer to hold the viewport matrix
    {
        const BufferOptions bufferOptions = {
            .label = "Viewport Buffer",
            .size = sizeof(glm::mat4),
            .usage = BufferUsageFlagBits::UniformBufferBit,
            .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
        };
        m_viewportBuffer = m_device.createBuffer(bufferOptions);
        m_viewportBufferData = m_viewportBuffer.map();

        // Set up initial viewport data
        updateViewportBuffer();
        std::memcpy(m_viewportBufferData, &m_viewportMatrix, sizeof(glm::mat4));
    }

    // Create a buffer to hold the material UBO
    {
        const BufferOptions bufferOptions = {
            .label = "Material Buffer",
            .size = sizeof(MaterialData),
            .usage = BufferUsageFlagBits::UniformBufferBit,
            .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
        };
        m_materialBuffer = m_device.createBuffer(bufferOptions);
        m_materialBufferData = m_materialBuffer.map();

        // Upload initial material data (red color)
        std::memcpy(m_materialBufferData, &m_materialData, sizeof(MaterialData));
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
        m_transformBufferData = m_transformBuffer.map();
        std::memcpy(m_transformBufferData, &m_transform, sizeof(glm::mat4));
    }

    // Create a vertex shader and fragment shader
    auto vertexShaderPath = KDGpuExample::assetDir().file("shaders/examples/wireframe_geometry/wireframe_geometry.vert.spv");
    auto vertexShader = m_device.createShaderModule(KDGpuExample::readShaderFile(vertexShaderPath));

    auto geometryShaderPath = KDGpuExample::assetDir().file("shaders/examples/wireframe_geometry/wireframe_geometry.geom.spv");
    auto geometryShader = m_device.createShaderModule(KDGpuExample::readShaderFile(geometryShaderPath));

    auto fragmentShaderPath = KDGpuExample::assetDir().file("shaders/examples/wireframe_geometry/wireframe_geometry.frag.spv");
    auto fragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(fragmentShaderPath));

    // Bind group layout set 0: camera UBO (vertex shader)
    // Bind group layout set 1: material UBO (fragment shader)
    // Bind group layout set 2: model transform UBO (vertex shader)

    // clang-format off
    const BindGroupLayoutOptions bindGroupLayoutOptionsSet0 = {
        .label = "Scene Data Bind Group Layout",
        .bindings = {{
            .binding = 0, // Camera UBO
            .resourceType = ResourceBindingType::UniformBuffer,
            .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit)
        },{
            .binding = 1, // Viewport UBO
            .resourceType = ResourceBindingType::UniformBuffer,
            .shaderStages = ShaderStageFlags(ShaderStageFlagBits::GeometryBit)
        }}
    };
    // clang-format on
    const BindGroupLayout bindGroupLayoutSet0 = m_device.createBindGroupLayout(bindGroupLayoutOptionsSet0);

    // clang-format off
    const BindGroupLayoutOptions bindGroupLayoutOptionsSet1 = {
        .label = "Material Bind Group Layout",
        .bindings = {{
            .binding = 0,
            .resourceType = ResourceBindingType::UniformBuffer,
            .shaderStages = ShaderStageFlags(ShaderStageFlagBits::FragmentBit)
        }}
    };
    // clang-format on
    const BindGroupLayout bindGroupLayoutSet1 = m_device.createBindGroupLayout(bindGroupLayoutOptionsSet1);

    // clang-format off
    const BindGroupLayoutOptions bindGroupLayoutOptionsSet2 = {
        .label = "Transform Bind Group",
        .bindings = {{
            .binding = 0,
            .resourceType = ResourceBindingType::UniformBuffer,
            .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit)
        }}
    };
    // clang-format on
    const BindGroupLayout bindGroupLayoutSet2 = m_device.createBindGroupLayout(bindGroupLayoutOptionsSet2);

    // Create a pipeline layout (array of bind group layouts)
    const PipelineLayoutOptions pipelineLayoutOptions = {
        .label = "Wireframe Geometry Pipeline Layout",
        .bindGroupLayouts = { bindGroupLayoutSet0, bindGroupLayoutSet1, bindGroupLayoutSet2 }
    };
    m_pipelineLayout = m_device.createPipelineLayout(pipelineLayoutOptions);

    // Create a pipeline
    // clang-format off
    const GraphicsPipelineOptions pipelineOptions = {
        .label = "Wireframe Geometry Shader Pipeline",
        .shaderStages = {
            { .shaderModule = vertexShader, .stage = ShaderStageFlagBits::VertexBit },
            { .shaderModule = geometryShader, .stage = ShaderStageFlagBits::GeometryBit },
            { .shaderModule = fragmentShader, .stage = ShaderStageFlagBits::FragmentBit }
        },
        .layout = m_pipelineLayout,
        .vertex = {
            .buffers = {
                { .binding = 0, .stride = sizeof(Vertex) }
            },
            .attributes = {
                { .location = 0, .binding = 0, .format = Format::R32G32B32_SFLOAT }, // Position
                { .location = 1, .binding = 0, .format = Format::R32G32B32_SFLOAT, .offset = offsetof(Vertex, normal) }, // Normal
                { .location = 2, .binding = 0, .format = Format::R32_SFLOAT, .offset = offsetof(Vertex, excludeEdge) } // ExcludeEdge
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
        .multisample = {
            .samples = m_samples.get()
        },
    };
    // clang-format on
    m_pipeline = m_device.createGraphicsPipeline(pipelineOptions);

    // Create bind groups for the camera UBO, material UBO and transform UBO
    {
        // clang-format off
        const BindGroupOptions bindGroupOptions = {
            .label = "Scene Data Bind Group",
            .layout = bindGroupLayoutSet0,
            .resources = {{
                .binding = 0,
                .resource = UniformBufferBinding{ .buffer = m_cameraBuffer }
            }, {
                .binding = 1,
                .resource = UniformBufferBinding{ .buffer = m_viewportBuffer }
            }}
        };
        // clang-format on
        m_cameraBindGroup = m_device.createBindGroup(bindGroupOptions);
    }
    {
        // clang-format off
        const BindGroupOptions bindGroupOptions = {
            .label = "Material Bind Group",
            .layout = bindGroupLayoutSet1,
            .resources = {{
                .binding = 0,
                .resource = UniformBufferBinding{ .buffer = m_materialBuffer }
            }}
        };
        // clang-format on
        m_materialBindGroup = m_device.createBindGroup(bindGroupOptions);
    }
    {
        // clang-format off
        const BindGroupOptions bindGroupOptions = {
            .label = "Transform Bind Group",
            .layout = bindGroupLayoutSet2,
            .resources = {{
                .binding = 0,
                .resource = UniformBufferBinding{ .buffer = m_transformBuffer }
            }}
        };
        // clang-format on
        m_transformBindGroup = m_device.createBindGroup(bindGroupOptions);
    }

    // Most of the render pass is the same between frames. The only thing that changes, is which image
    // of the swapchain we wish to render to. So set up what we can here, and in the render loop we will
    // just update the color texture view.
    // clang-format off
    m_opaquePassOptions = {
        .colorAttachments = {
            {
                .view = m_msaaTextureView, // We render to a multisampled texture, not directly to the swapchain image
                .resolveView = {}, // Not setting the swapchain texture view just yet
                .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f },
                .finalLayout = TextureLayout::PresentSrc
            }
        },
        .depthStencilAttachment = {
            .view = m_depthTextureView,
        },
        .samples = m_samples.get()
    };
    // clang-format on

    // Create a multisample texture into which we will render. The pipeline will then resolve the
    // multi-sampled texture into the current swapchain image.
    createRenderTarget();

    registerImGuiOverlayDrawFunction([this](ImGuiContext *ctx) {
        drawControls(ctx);
    });
}

void WireframeGeometry::cleanupScene()
{
    m_pipeline = {};
    m_pipelineLayout = {};
    m_vertexBuffer = {};
    m_transformBindGroup = {};
    m_transformBuffer = {};
    m_transformBufferData = nullptr;
    m_materialBindGroup = {};
    m_materialBuffer = {};
    m_materialBufferData = nullptr;
    m_cameraBindGroup = {};
    m_cameraBuffer = {};
    m_cameraBufferData = nullptr;
    m_viewportBindGroup = {};
    m_viewportBuffer = {};
    m_viewportBufferData = nullptr;
    m_commandBuffer = {};
    m_msaaTextureView = {};
    m_msaaTexture = {};
}

void WireframeGeometry::createRenderTarget()
{
    // Reset depthTextureView as depthStencilAttachment view as it might
    // have been recreated following a resize
    m_opaquePassOptions.depthStencilAttachment.view = m_depthTextureView;

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
        m_opaquePassOptions.colorAttachments[0].view = m_msaaTextureView;
}

bool WireframeGeometry::isMsaaEnabled() const
{
    return m_samples.get() != SampleCountFlagBits::Samples1Bit;
}

void WireframeGeometry::updateScene()
{
    // Each frame we want to rotate the triangle a little
    static float angle = 0.0f;
    const float angularSpeed = 6.0f; // degrees per second
    const float dt = engine()->deltaTimeSeconds();
    angle += angularSpeed * dt;
    if (angle > 360.0f)
        angle -= 360.0f;

    m_transform = glm::mat4(1.0f);
    m_transform = glm::rotate(m_transform, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    std::memcpy(m_transformBufferData, &m_transform, sizeof(glm::mat4));

    // If the viewport has changed (window resize), update the viewport matrix UBO
    if (m_viewportDirty) {
        std::memcpy(m_viewportBufferData, &m_viewportMatrix, sizeof(glm::mat4));
        m_viewportDirty = false;
    }

    // If the material has changed (via ImGui), update the material UBO
    if (m_materialDirty) {
        std::memcpy(m_materialBufferData, &m_materialData, sizeof(MaterialData));
        m_materialDirty = false;
    }
}

void WireframeGeometry::updateViewportBuffer()
{
    const float w2 = float(m_swapchainExtent.width) / 2.0f;
    const float h2 = float(m_swapchainExtent.height) / 2.0f;
    m_viewportMatrix = glm::mat4(
            w2, 0.0f, 0.0f, 0.0f,
            0.0f, -h2, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            w2, h2, 0.0f, 1.0f);
    m_viewportDirty = true;
}

void WireframeGeometry::drawControls(ImGuiContext *ctx)
{
    constexpr ImVec2 winOffset(10, 180);

    ImGui::SetCurrentContext(ctx);
    ImGui::SetNextWindowPos(winOffset);
    ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::Begin("Controls",
                 nullptr,
                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);

    ImGui::Text("Wireframe Color and Width");
    if (ImGui::ColorEdit3("Wireframe Color", &m_materialData.wireframeColorAndWidth.r))
        m_materialDirty = true;
    if (ImGui::SliderFloat("Wireframe Width (pixels)", &m_materialData.wireframeColorAndWidth.w, 1.0f, 10.0f))
        m_materialDirty = true;
    ImGui::Spacing();

    ImGui::Text("Base Color");
    if (ImGui::ColorEdit3("Base Color", &m_materialData.baseColorFactor.r))
        m_materialDirty = true;
    ImGui::Spacing();

    ImGui::Text("Wireframe Gradient (world space height)");
    if (ImGui::SliderFloat("Gradient Start", &m_materialData.wireframeGradient.x, -2.0f, 2.0f)) {
        m_materialDirty = true;
        if (m_materialData.wireframeGradient.x > m_materialData.wireframeGradient.y)
            m_materialData.wireframeGradient.y = m_materialData.wireframeGradient.x;
    }

    if (ImGui::SliderFloat("Gradient End", &m_materialData.wireframeGradient.y, -2.0f, 2.0f)) {
        m_materialDirty = true;
        if (m_materialData.wireframeGradient.y < m_materialData.wireframeGradient.x)
            m_materialData.wireframeGradient.x = m_materialData.wireframeGradient.y;
    }
    ImGui::Spacing();

    ImGui::End();
}

void WireframeGeometry::resize()
{
    // Update the render target (MSAA texture if enabled)
    createRenderTarget();

    // Update the viewport matrix as well
    updateViewportBuffer();
}

void WireframeGeometry::render()
{
    auto commandRecorder = m_device.createCommandRecorder();

    m_opaquePassOptions.colorAttachments[0].resolveView = m_swapchainViews.at(m_currentSwapchainImageIndex);
    auto opaquePass = commandRecorder.beginRenderPass(m_opaquePassOptions);

    opaquePass.setPipeline(m_pipeline);
    opaquePass.setVertexBuffer(0, m_vertexBuffer);
    opaquePass.setBindGroup(0, m_cameraBindGroup);
    opaquePass.setBindGroup(1, m_materialBindGroup);
    opaquePass.setBindGroup(2, m_transformBindGroup);
    const DrawCommand drawCmd = { .vertexCount = 36 };
    opaquePass.draw(drawCmd);
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

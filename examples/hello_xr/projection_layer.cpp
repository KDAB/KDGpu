/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "projection_layer.h"

#include <KDGpuExample/engine.h>
#include <KDGpuExample/kdgpuexample.h>
#include <KDGpuExample/view_projection.h>
#include <KDGpuExample/xr_example_engine_layer.h>

#include <KDGpu/bind_group_layout_options.h>
#include <KDGpu/bind_group_options.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/graphics_pipeline_options.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include <array>

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

ProjectionLayer::ProjectionLayer(const XrProjectionLayerOptions &options)
    : XrProjectionLayer(options)
{
}

ProjectionLayer::~ProjectionLayer()
{
}

void ProjectionLayer::initializeScene()
{
    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
    };

    // Create a buffer to hold triangle vertex data
    {
        // This is in model space which is y-up in this example. Note this is upside down vs the hello_triangle example which
        // draws the triangle directly in NDC space which is y-down.
        const float r = 0.8f;
        const std::array<Vertex, 3> vertexData = {
            Vertex{ // Bottom-left, red
                    .position = { r * cosf(7.0f * M_PI / 6.0f), r * sinf(7.0f * M_PI / 6.0f), 0.0f },
                    .color = { 1.0f, 0.4f, 0.3f } },
            Vertex{ // Bottom-right, green
                    .position = { r * cosf(11.0f * M_PI / 6.0f), r * sinf(11.0f * M_PI / 6.0f), 0.0f },
                    .color = { 0.7f, 1.0f, 0.3f } },
            Vertex{ // Top, blue
                    .position = { 0.0f, r, 0.0f },
                    .color = { 0.5f, 0.2f, 1.0f } }
        };

        const DeviceSize dataByteSize = vertexData.size() * sizeof(Vertex);
        const BufferOptions bufferOptions = {
            .label = "Main Triangle Vertex Buffer",
            .size = dataByteSize,
            .usage = BufferUsageFlagBits::VertexBufferBit | BufferUsageFlagBits::TransferDstBit,
            .memoryUsage = MemoryUsage::GpuOnly
        };
        m_buffer = m_device->createBuffer(bufferOptions);
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
        const BufferOptions bufferOptions = {
            .label = "Index Buffer",
            .size = dataByteSize,
            .usage = BufferUsageFlagBits::IndexBufferBit | BufferUsageFlagBits::TransferDstBit,
            .memoryUsage = MemoryUsage::GpuOnly
        };
        m_indexBuffer = m_device->createBuffer(bufferOptions);
        const BufferUploadOptions uploadOptions = {
            .destinationBuffer = m_indexBuffer,
            .dstStages = PipelineStageFlagBit::IndexInputBit,
            .dstMask = AccessFlagBit::IndexReadBit,
            .data = indexData.data(),
            .byteSize = dataByteSize
        };
        uploadBufferData(uploadOptions);
    }

    // Create a buffer to hold triangle vertex data for the left controller
    {
        const std::array<Vertex, 3> vertexData = {
            Vertex{ // Back-left, red
                    .position = { -0.05f, 0.0f, 0.0f },
                    .color = { 1.0f, 0.0f, 0.0f } },
            Vertex{ // Back-right, red
                    .position = { 0.05f, 0.0f, 0.0f },
                    .color = { 1.0f, 0.0f, 0.0f } },
            Vertex{ // Front-center, red
                    .position = { 0.0f, 0.0f, -0.2f },
                    .color = { 1.0f, 0.0f, 0.0f } }
        };

        const DeviceSize dataByteSize = vertexData.size() * sizeof(Vertex);
        const BufferOptions bufferOptions = {
            .label = "Left Hand Triangle Vertex Buffer",
            .size = dataByteSize,
            .usage = BufferUsageFlagBits::VertexBufferBit | BufferUsageFlagBits::TransferDstBit,
            .memoryUsage = MemoryUsage::GpuOnly
        };
        m_leftHandBuffer = m_device->createBuffer(bufferOptions);
        const BufferUploadOptions uploadOptions = {
            .destinationBuffer = m_leftHandBuffer,
            .dstStages = PipelineStageFlagBit::VertexAttributeInputBit,
            .dstMask = AccessFlagBit::VertexAttributeReadBit,
            .data = vertexData.data(),
            .byteSize = dataByteSize
        };
        uploadBufferData(uploadOptions);
    }

    // Create a buffer to hold triangle vertex data for the left controller
    {
        const std::array<Vertex, 3> vertexData = {
            Vertex{ // Back-left, blue
                    .position = { -0.05f, 0.0f, 0.0f },
                    .color = { 0.0f, 0.0f, 1.0f } },
            Vertex{ // Back-right, blue
                    .position = { 0.05f, 0.0f, 0.0f },
                    .color = { 0.0f, 0.0f, 1.0f } },
            Vertex{ // Front-center, blue
                    .position = { 0.0f, 0.0f, -0.2f },
                    .color = { 0.0f, 0.0f, 1.0f } }
        };

        const DeviceSize dataByteSize = vertexData.size() * sizeof(Vertex);
        const BufferOptions bufferOptions = {
            .label = "Right Hand Triangle Vertex Buffer",
            .size = dataByteSize,
            .usage = BufferUsageFlagBits::VertexBufferBit | BufferUsageFlagBits::TransferDstBit,
            .memoryUsage = MemoryUsage::GpuOnly
        };
        m_rightHandBuffer = m_device->createBuffer(bufferOptions);
        const BufferUploadOptions uploadOptions = {
            .destinationBuffer = m_rightHandBuffer,
            .dstStages = PipelineStageFlagBit::VertexAttributeInputBit,
            .dstMask = AccessFlagBit::VertexAttributeReadBit,
            .data = vertexData.data(),
            .byteSize = dataByteSize
        };
        uploadBufferData(uploadOptions);
    }

    // Create a buffer to hold the entity transformation matrix
    {
        const BufferOptions bufferOptions = {
            .label = "Transformation Buffer",
            .size = sizeof(glm::mat4),
            .usage = BufferUsageFlagBits::UniformBufferBit,
            .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
        };
        m_transformBuffer = m_device->createBuffer(bufferOptions);

        // Upload identity matrix. Updated below in updateScene()
        m_transform = glm::mat4(1.0f);
        auto bufferData = m_transformBuffer.map();
        std::memcpy(bufferData, &m_transform, sizeof(glm::mat4));
        m_transformBuffer.unmap();
    }

    // Create a buffer to hold the left hand transformation matrix
    {
        const BufferOptions bufferOptions = {
            .label = "Left Hand Transformation Buffer",
            .size = sizeof(glm::mat4),
            .usage = BufferUsageFlagBits::UniformBufferBit,
            .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
        };
        m_leftHandTransformBuffer = m_device->createBuffer(bufferOptions);

        // Upload identity matrix. Updated below in updateScene()
        m_leftHandTransform = glm::mat4(1.0f);
        auto bufferData = m_leftHandTransformBuffer.map();
        std::memcpy(bufferData, &m_leftHandTransform, sizeof(glm::mat4));
        m_leftHandTransformBuffer.unmap();
    }

    // Create a buffer to hold the right hand transformation matrix
    {
        const BufferOptions bufferOptions = {
            .label = "Right Hand Transformation Buffer",
            .size = sizeof(glm::mat4),
            .usage = BufferUsageFlagBits::UniformBufferBit,
            .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
        };
        m_rightHandTransformBuffer = m_device->createBuffer(bufferOptions);

        // Upload identity matrix. Updated below in updateScene()
        m_rightHandTransform = glm::mat4(1.0f);
        auto bufferData = m_rightHandTransformBuffer.map();
        std::memcpy(bufferData, &m_rightHandTransform, sizeof(glm::mat4));
        m_rightHandTransformBuffer.unmap();
    }

    // Create a buffer to hold the camera view and projection matrices
    {
        const BufferOptions bufferOptions = {
            .label = "Camera Buffer",
            .size = sizeof(glm::mat4) * 2,
            .usage = BufferUsageFlagBits::UniformBufferBit,
            .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
        };
        m_cameraBuffer = m_device->createBuffer(bufferOptions);

        // Upload identity matrices. Updated below in updateScene()
        glm::mat4 viewMatrix(1.0f);
        glm::mat4 projectionMatrix(1.0f);
        auto bufferData = static_cast<float *>(m_cameraBuffer.map());
        std::memcpy(bufferData, glm::value_ptr(viewMatrix), sizeof(glm::mat4));
        std::memcpy(bufferData + 16, glm::value_ptr(projectionMatrix), sizeof(glm::mat4));
        m_cameraBuffer.unmap();
    }

    // Create a vertex shader and fragment shader
    const auto vertexShaderPath = KDGpu::assetPath() + "/shaders/examples/hello_xr/hello_xr.vert.spv";
    auto vertexShader = m_device->createShaderModule(KDGpuExample::readShaderFile(vertexShaderPath));

    const auto fragmentShaderPath = KDGpu::assetPath() + "/shaders/examples/hello_xr/hello_xr.frag.spv";
    auto fragmentShader = m_device->createShaderModule(KDGpuExample::readShaderFile(fragmentShaderPath));

    // Create bind group layout consisting of a single binding holding a UBO for the entity transform
    // clang-format off
    const BindGroupLayoutOptions entityBindGroupLayoutOptions = {
        .label = "Entity Transform Bind Group",
        .bindings = {{
            .binding = 0,
            .resourceType = ResourceBindingType::UniformBuffer,
            .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit)
        }}
    };
    // clang-format on
    const BindGroupLayout entityBindGroupLayout = m_device->createBindGroupLayout(entityBindGroupLayoutOptions);

    // Create bind group layout consisting of a single binding holding a UBO for the camera view and projection matrices
    // clang-format off
    const BindGroupLayoutOptions cameraBindGroupLayoutOptions = {
        .label = "Camera Transform Bind Group",
        .bindings = {{
            .binding = 0,
            .resourceType = ResourceBindingType::UniformBuffer,
            .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit)
        }}
    };
    // clang-format on
    const BindGroupLayout cameraBindGroupLayout = m_device->createBindGroupLayout(cameraBindGroupLayoutOptions);

    // Create a pipeline layout (array of bind group layouts)
    const PipelineLayoutOptions pipelineLayoutOptions = {
        .label = "Triangle",
        .bindGroupLayouts = { entityBindGroupLayout, cameraBindGroupLayout }
    };
    m_pipelineLayout = m_device->createPipelineLayout(pipelineLayoutOptions);

    // Create a pipeline
    // clang-format off
    const GraphicsPipelineOptions pipelineOptions = {
        .label = "Triangle",
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
            { .format = m_colorSwapchainFormat }
        },
        .depthStencil = {
            .format = m_depthSwapchainFormat,
            .depthWritesEnabled = true,
            .depthCompareOperation = CompareOperation::Less
        },
        .primitive = {
            .cullMode = CullModeFlagBits::None
        }
    };
    // clang-format on
    m_pipeline = m_device->createGraphicsPipeline(pipelineOptions);

    // Create a bindGroup to hold the UBO with the entity transform
    // clang-format off
    const BindGroupOptions entityBindGroupOptions = {
        .label = "Transform Bind Group",
        .layout = entityBindGroupLayout,
        .resources = {{
            .binding = 0,
            .resource = UniformBufferBinding{ .buffer = m_transformBuffer }
        }}
    };
    // clang-format on
    m_entityTransformBindGroup = m_device->createBindGroup(entityBindGroupOptions);

    // Create a bindGroup to hold the UBO with the left hand transform
    // clang-format off
    const BindGroupOptions leftHandBindGroupOptions = {
        .label = "Left Hand Transform Bind Group",
        .layout = entityBindGroupLayout,
        .resources = {{
            .binding = 0,
            .resource = UniformBufferBinding{ .buffer = m_leftHandTransformBuffer }
        }}
    };
    // clang-format on
    m_leftHandTransformBindGroup = m_device->createBindGroup(leftHandBindGroupOptions);

    // Create a bindGroup to hold the UBO with the right hand transform
    // clang-format off
    const BindGroupOptions rightHandBindGroupOptions = {
        .label = "Right Hand Transform Bind Group",
        .layout = entityBindGroupLayout,
        .resources = {{
            .binding = 0,
            .resource = UniformBufferBinding{ .buffer = m_rightHandTransformBuffer }
        }}
    };
    // clang-format on
    m_rightHandTransformBindGroup = m_device->createBindGroup(rightHandBindGroupOptions);

    // Create a bindGroup to hold the UBO with the camera view and projection matrices
    // clang-format off
    const BindGroupOptions cameraBindGroupOptions = {
        .label = "Camera Bind Group",
        .layout = cameraBindGroupLayout,
        .resources = {{
            .binding = 0,
            .resource = UniformBufferBinding{ .buffer = m_cameraBuffer }
        }}
    };
    // clang-format on
    m_cameraBindGroup = m_device->createBindGroup(cameraBindGroupOptions);

    // Most of the render pass is the same between frames. The only thing that changes, is which image
    // of the swapchain we wish to render to. So set up what we can here, and in the render loop we will
    // just update the color texture view.
    // clang-format off
    m_opaquePassOptions = {
        .colorAttachments = {
            {
                .view = {}, // Not setting the swapchain texture view just yet
                .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f },
                .finalLayout = TextureLayout::ColorAttachmentOptimal
            }
        },
        .depthStencilAttachment = {
            .view = {} // Not setting the depth texture view just yet
        }
    };
    // clang-format on

    // We will use a fence to synchronize CPU and GPU. When we render image for each view (eye), we
    // shall wait for the fence to be signaled before we update any shared resources such as a view
    // matrix UBO (not used yet). An alternative would be to index into an array of such matrices.
    m_fence = m_device->createFence({ .label = "Projection Layer Scene Fence" });
}

void ProjectionLayer::cleanupScene()
{
    m_fence = {};

    m_cameraBindGroup = {};
    m_cameraBuffer = {};

    m_rightHandTransformBindGroup = {};
    m_rightHandTransformBuffer = {};
    m_rightHandBuffer = {};
    m_leftHandTransformBindGroup = {};
    m_leftHandTransformBuffer = {};
    m_leftHandBuffer = {};

    m_pipeline = {};
    m_pipelineLayout = {};
    m_buffer = {};
    m_indexBuffer = {};
    m_entityTransformBindGroup = {};
    m_transformBuffer = {};
    m_commandBuffer = {};
}

void ProjectionLayer::initialize()
{
    XrProjectionLayer::initialize();
    initializeScene();
}

void ProjectionLayer::cleanup()
{
    cleanupScene();
    XrProjectionLayer::cleanup();
}

// In this function we will update our local copy of the view matrices and transform
// data. Note that we do not update the UBOs here as the GPU may still be reading from
// them at this stage. We cannot be sure it is safe to update the GPU data until we
// have waited for the fence to be signaled in the renderView() function.
void ProjectionLayer::updateScene()
{
    // Update the camera data for each view
    m_cameraData.resize(m_viewState.viewCount());
    for (uint32_t viewIndex = 0; viewIndex < m_viewState.viewCount(); ++viewIndex) {
        const auto &view = m_viewState.views[viewIndex];
        const KDXr::Quaternion &orientation = view.pose.orientation;
        const KDXr::Vector3 &position = view.pose.position;

        // clang-format off
        m_cameraData[viewIndex].view = viewMatrix({
            .orientation = glm::quat(orientation.w, orientation.x, orientation.y, orientation.z),
            .position = glm::vec3(position.x, position.y, position.z)
        });
        m_cameraData[viewIndex].projection = perspective({
            .leftFieldOfView = m_viewState.views[viewIndex].fieldOfView.angleLeft,
            .rightFieldOfView = m_viewState.views[viewIndex].fieldOfView.angleRight,
            .upFieldOfView = m_viewState.views[viewIndex].fieldOfView.angleUp,
            .downFieldOfView = m_viewState.views[viewIndex].fieldOfView.angleDown,
            .nearPlane = m_nearPlane,
            .farPlane = m_farPlane,
            .applyPostViewCorrection = ApplyPostViewCorrection::Yes
        });
        // clang-format on
    }

    // Scale the triangle up and down
    const float s = scale();

    // If we are animating, each frame we want to rotate the triangle a little
    static float angle = 0.0f;
    if (rotateZ()) {
        const float angularSpeed = 10.0f; // degrees per second
        const float dt = engine()->deltaTimeSeconds();
        angle += angularSpeed * dt;
        if (angle > 360.0f)
            angle -= 360.0f;
    }

    static float rotateYAngle = 0.0f;
    if (rotateY()) {
        const float angularSpeed = 10.0f; // degrees per second
        const float dt = engine()->deltaTimeSeconds();
        rotateYAngle += angularSpeed * dt;
        if (rotateYAngle > 360.0f)
            rotateYAngle -= 360.0f;
    }

    m_transform = glm::mat4(1.0f);
    m_transform = glm::translate(m_transform, translation());
    m_transform = glm::rotate(m_transform, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
    m_transform = glm::rotate(m_transform, glm::radians(rotateYAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    m_transform = glm::scale(m_transform, glm::vec3(s, s, s));

    // Update the transformation matrix for the left hand from the pose
    {
        const auto &orientation = leftPalmPose().orientation;
        glm::quat q(orientation.w, orientation.x, orientation.y, orientation.z);
        glm::mat4 mRot = glm::toMat4(q);
        const auto &position = leftPalmPose().position;
        glm::vec3 p(position.x, position.y, position.z);
        glm::mat4 mTrans = glm::translate(glm::mat4(1.0f), p);
        m_leftHandTransform = mTrans * mRot;
    }

    // Update the transformation matrix for the right hand from the pose
    {
        const auto &orientation = rightPalmPose().orientation;
        glm::quat q(orientation.w, orientation.x, orientation.y, orientation.z);
        glm::mat4 mRot = glm::toMat4(q);
        const auto &position = rightPalmPose().position;
        glm::vec3 p(position.x, position.y, position.z);
        glm::mat4 mTrans = glm::translate(glm::mat4(1.0f), p);
        m_rightHandTransform = mTrans * mRot;
    }
}

void ProjectionLayer::updateTransformUbo()
{
    auto bufferData = m_transformBuffer.map();
    std::memcpy(bufferData, &m_transform, sizeof(glm::mat4));
    m_transformBuffer.unmap();

    bufferData = m_leftHandTransformBuffer.map();
    std::memcpy(bufferData, &m_leftHandTransform, sizeof(glm::mat4));
    m_leftHandTransformBuffer.unmap();

    bufferData = m_rightHandTransformBuffer.map();
    std::memcpy(bufferData, &m_rightHandTransform, sizeof(glm::mat4));
    m_rightHandTransformBuffer.unmap();
}

void ProjectionLayer::updateViewUbo()
{
    auto cameraBufferData = static_cast<float *>(m_cameraBuffer.map());
    std::memcpy(cameraBufferData, m_cameraData.data() + m_currentViewIndex, sizeof(CameraData));
    m_cameraBuffer.unmap();
}

void ProjectionLayer::renderView()
{
    m_fence.wait();
    m_fence.reset();

    // Update the scene data once per frame
    if (m_currentViewIndex == 0) {
        updateTransformUbo();
    }

    // Update the per-view camera matrices
    updateViewUbo();

    auto commandRecorder = m_device->createCommandRecorder();

    // Set up the render pass using the current color and depth texture views
    m_opaquePassOptions.colorAttachments[0].view = m_colorSwapchains[m_currentViewIndex].textureViews[m_currentColorImageIndex];
    m_opaquePassOptions.depthStencilAttachment.view = m_depthSwapchains[m_currentViewIndex].textureViews[m_currentDepthImageIndex];
    auto opaquePass = commandRecorder.beginRenderPass(m_opaquePassOptions);

    // Draw the main triangle
    opaquePass.setPipeline(m_pipeline);
    opaquePass.setVertexBuffer(0, m_buffer);
    opaquePass.setIndexBuffer(m_indexBuffer);
    opaquePass.setBindGroup(0, m_cameraBindGroup);
    opaquePass.setBindGroup(1, m_entityTransformBindGroup);
    const DrawIndexedCommand drawCmd = { .indexCount = 3 };
    opaquePass.drawIndexed(drawCmd);

    // Draw the left hand triangle
    opaquePass.setVertexBuffer(0, m_leftHandBuffer);
    opaquePass.setBindGroup(1, m_leftHandTransformBindGroup);
    opaquePass.drawIndexed(drawCmd);

    // Draw the right hand triangle
    opaquePass.setVertexBuffer(0, m_rightHandBuffer);
    opaquePass.setBindGroup(1, m_rightHandTransformBindGroup);
    opaquePass.drawIndexed(drawCmd);

    opaquePass.end();
    m_commandBuffer = commandRecorder.finish();

    const SubmitOptions submitOptions = {
        .commandBuffers = { m_commandBuffer },
        .signalFence = m_fence
    };
    m_queue->submit(submitOptions);
}

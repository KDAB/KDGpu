/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "xr_quad_imgui_layer.h"

#include <KDGpuExample/engine.h>
#include <KDGpuExample/imgui_input_handler.h>
#include <KDGpuExample/imgui_item.h>
#include <KDGpuExample/imgui_renderer.h>
#include <KDGpuExample/xr_example_engine_layer.h>

#include <KDGpu/adapter.h>
#include <KDGpu/device.h>
#include <KDGpu/render_pass_command_recorder.h>
#include <KDGpu/texture_options.h>
#include <KDGui/gui_application.h>

#include <imgui.h>

namespace KDGpuExample {

XrQuadImGuiLayer::XrQuadImGuiLayer(const XrQuadLayerOptions &options)
    : XrQuadLayer(options)
{
}

XrQuadImGuiLayer::~XrQuadImGuiLayer()
{
}

void XrQuadImGuiLayer::initialize()
{
    XrQuadLayer::initialize();

    // clang-format off
    m_imguiPassOptions = {
        .colorAttachments = {
            {
                .view = {}, // Not setting the swapchain texture view just yet
                .clearValue = { 0.0f, 0.0f, 0.0f, 0.7f },
                .finalLayout = TextureLayout::ColorAttachmentOptimal
            }
        },
        .depthStencilAttachment = {
            .view = {} // Not setting the depth texture view just yet
        }
    };
    // clang-format on

    // Use a fence to stop us trampling on frames in flight
    m_fence = m_device->createFence({ .label = "ImGui Fence" });

    recreateImGuiOverlay();
}

void XrQuadImGuiLayer::cleanup()
{
    m_imguiOverlay = {};
    XrQuadLayer::cleanup();
}

void XrQuadImGuiLayer::renderQuad()
{
    updateImGuiOverlay();

    m_fence.wait();
    m_fence.reset();

    auto commandRecorder = m_device->createCommandRecorder();

    // Set up the render pass using the current color and depth texture views
    m_imguiPassOptions.colorAttachments[0].view = m_colorSwapchain.textureViews[m_currentColorImageIndex];
    m_imguiPassOptions.depthStencilAttachment.view = m_depthSwapchain.textureViews[m_currentDepthImageIndex];
    auto imguiPass = commandRecorder.beginRenderPass(m_imguiPassOptions);

    renderImGuiOverlay(&imguiPass, m_currentColorImageIndex);

    imguiPass.end();
    m_commandBuffer = commandRecorder.finish();

    const SubmitOptions submitOptions = {
        .commandBuffers = { m_commandBuffer },
        .signalFence = m_fence
    };
    m_queue->submit(submitOptions);
}

void XrQuadImGuiLayer::drawImGuiOverlay(ImGuiContext *ctx)
{
    ImGui::SetCurrentContext(ctx);
    ImGui::SetNextWindowPos(ImVec2(10, 20));
    ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::Begin(
            "Basic Info",
            nullptr,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);

    ImGui::Text("App: %s", KDGui::GuiApplication::instance()->applicationName().data());
    ImGui::Text("GPU: %s", m_device->adapter()->properties().deviceName.c_str());
    const auto fps = m_engineLayer->engine()->fps();
    ImGui::Text("%.2f ms/frame (%.1f fps)", (1000.0f / fps), fps);
    ImGui::End();

    for (const auto &func : m_imGuiOverlayDrawFunctions)
        func(ctx);
}

void XrQuadImGuiLayer::renderImGuiOverlay(RenderPassCommandRecorder *recorder, uint32_t inFlightIndex)
{
    // Updates the geometry buffers used by ImGui and records the commands needed to
    // get the ui into a render target.
    m_imguiOverlay->render(recorder, resolution(), inFlightIndex);
}

void XrQuadImGuiLayer::registerImGuiOverlayDrawFunction(const std::function<void(ImGuiContext *)> &func)
{
    m_imGuiOverlayDrawFunctions.push_back(func);
}

void XrQuadImGuiLayer::clearImGuiOverlayDrawFunctions()
{
    m_imGuiOverlayDrawFunctions.clear();
}

void XrQuadImGuiLayer::recreateImGuiOverlay()
{
    m_imguiOverlay = std::make_unique<ImGuiItem>(m_device, m_queue);
    m_imguiOverlay->initialize(1.0f, m_samples, m_colorSwapchainFormat, m_depthSwapchainFormat);
}

void XrQuadImGuiLayer::updateImGuiOverlay()
{
    ImGuiContext *context = m_imguiOverlay->context();
    ImGui::SetCurrentContext(context);

    // Set frame time and display size.
    ImGuiIO &io = ImGui::GetIO();
    io.DeltaTime = m_engineLayer->engine()->deltaTimeSeconds();
    io.DisplaySize = ImVec2(static_cast<float>(resolution().width), static_cast<float>(resolution().height));

    // Call our imgui drawing function
    ImGui::NewFrame();
    drawImGuiOverlay(context);

    // Process the ImGui drawing functions to generate geometry and commands. The actual buffers will be updated
    // and commands translated by the ImGuiRenderer later in the frame.
    ImGui::Render();
}

} // namespace KDGpuExample

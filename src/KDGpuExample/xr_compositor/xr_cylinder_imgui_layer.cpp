/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "xr_cylinder_imgui_layer.h"

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

XrCylinderImGuiLayer::XrCylinderImGuiLayer(const XrCylinderLayerOptions &options)
    : XrCylinderLayer(options)
{
    // Add some interesting UI by default
    // can be removed with clearImGuiOverlayDrawFunctions
    registerImGuiOverlayDrawFunction([this](ImGuiContext *ctx) {
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

        ImColor bgEdit = { backgroundColor().float32[0], backgroundColor().float32[1], backgroundColor().float32[2], backgroundColor().float32[3] };
        if (ImGui::ColorEdit4("Background Color", (float *)&bgEdit, ImGuiColorEditFlags_NoInputs)) {
            backgroundColor = { bgEdit.Value.x, bgEdit.Value.y, bgEdit.Value.z, bgEdit.Value.w };
        }
        ImGui::End();
    });

    registerImGuiOverlayDrawFunction(drawMouseCursor);

    (void)backgroundColor.valueChanged().connect([this]() { setupRenderPassOptions(); });
}

XrCylinderImGuiLayer::~XrCylinderImGuiLayer()
{
}

void XrCylinderImGuiLayer::initialize()
{
    XrCylinderLayer::initialize();

    setupRenderPassOptions();

    // Use a fence to stop us trampling on frames in flight
    m_fence = m_device->createFence({ .label = "ImGui Fence" });

    recreateImGuiOverlay();
}

void XrCylinderImGuiLayer::cleanup()
{
    m_imguiOverlay = {};
    XrCylinderLayer::cleanup();
}

void XrCylinderImGuiLayer::renderCylinder()
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

void XrCylinderImGuiLayer::drawImGuiOverlay(ImGuiContext *ctx)
{
    ImGui::SetCurrentContext(ctx);

    for (const auto &func : m_imGuiOverlayDrawFunctions)
        func(ctx);
}

void XrCylinderImGuiLayer::renderImGuiOverlay(RenderPassCommandRecorder *recorder, uint32_t inFlightIndex)
{
    // Updates the geometry buffers used by ImGui and records the commands needed to
    // get the ui into a render target.
    m_imguiOverlay->render(recorder, resolution(), inFlightIndex);
}

void XrCylinderImGuiLayer::registerImGuiOverlayDrawFunction(const std::function<void(ImGuiContext *)> &func)
{
    m_imGuiOverlayDrawFunctions.push_back(func);
}

void XrCylinderImGuiLayer::clearImGuiOverlayDrawFunctions()
{
    m_imGuiOverlayDrawFunctions.clear();
}

void XrCylinderImGuiLayer::recreateImGuiOverlay()
{
    m_imguiOverlay = std::make_unique<ImGuiItem>(m_device, m_queue);
    m_imguiOverlay->initialize(1.0f, m_samples, m_colorSwapchainFormat, m_depthSwapchainFormat);
}

void XrCylinderImGuiLayer::updateImGuiOverlay()
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

ImGuiItem &XrCylinderImGuiLayer::overlay()
{
    assert(m_imguiOverlay);
    return *m_imguiOverlay;
}

void XrCylinderImGuiLayer::setupRenderPassOptions()
{
    // clang-format off
    m_imguiPassOptions = {
        .colorAttachments = {
            {
                .view = {}, // Not setting the swapchain texture view just yet
                .clearValue = backgroundColor(),
                .finalLayout = TextureLayout::ColorAttachmentOptimal
            }
        },
        .depthStencilAttachment = {
            .view = {} // Not setting the depth texture view just yet
        }
    };
    // clang-format on
}

void XrCylinderImGuiLayer::drawMouseCursor(ImGuiContext *ctx)
{
    ImGui::SetCurrentContext(ctx);

    ImGui::Begin(
            "Mouse Cursor",
            nullptr,
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);

    // Get the current mouse position
    const auto mousePos = ImGui::GetMousePos();

    // Define the custom cursor size
    const auto cursorHeight = ImGui::GetFrameHeight();
    const auto cursorWidth = cursorHeight / 1.5f;

    // Calculate the vertices for the rotated triangle pointing top-left
    const auto p1 = ImVec2(mousePos.x, mousePos.y); // Tip of the triangle
    auto dir2 = ImVec2(cursorWidth, cursorHeight);
    const auto dir2Mag = sqrtf(dir2.x * dir2.x + dir2.y * dir2.y);
    dir2.x = dir2.x * cursorHeight / dir2Mag;
    dir2.y = dir2.y * cursorHeight / dir2Mag;

    const auto p2 = ImVec2(p1.x + dir2.x, p1.y + dir2.y);
    const auto p3 = ImVec2(mousePos.x, mousePos.y + cursorHeight); // Bottom-left corner

    // Draw the black outline of the cursor
    auto *drawList = ImGui::GetForegroundDrawList();
    drawList->AddTriangle(
            p1, p2, p3,
            ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 1.0f)), // Black color
            2.0f // Thickness of the outline
    );

    // Draw the white filled triangle for the cursor
    drawList->AddTriangleFilled(
            p1, p2, p3,
            ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)) // White color
    );

    ImGui::End();
}
} // namespace KDGpuExample

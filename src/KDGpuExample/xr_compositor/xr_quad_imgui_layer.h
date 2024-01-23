/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpuExample/xr_compositor/xr_quad_layer.h>
#include <KDGpuExample/kdgpuexample_export.h>

#include <KDGpu/command_buffer.h>
#include <KDGpu/fence.h>
#include <KDGpu/render_pass_command_recorder_options.h>

struct ImGuiContext;

namespace KDGpu {
class RenderPassCommandRecorder;
}

namespace KDGpuExample {

class ImGuiItem;

// TODO: Can we make the ImGui content a mix-in class so we can also use it for cylinder layers?
class KDGPUEXAMPLE_EXPORT XrQuadImGuiLayer : public XrQuadLayer
{
public:
    explicit XrQuadImGuiLayer(const XrQuadLayerOptions &options);
    ~XrQuadImGuiLayer() override;

    void registerImGuiOverlayDrawFunction(const std::function<void(ImGuiContext *)> &func);
    void clearImGuiOverlayDrawFunctions();

protected:
    void renderQuad() override;

    // TODO: Can we share this with ExampleEngineLayer or any other ImGui renderer?
    virtual void drawImGuiOverlay(ImGuiContext *ctx);
    virtual void renderImGuiOverlay(KDGpu::RenderPassCommandRecorder *recorder, uint32_t inFlightIndex = 0);

    void recreateImGuiOverlay();
    void updateImGuiOverlay();

    std::unique_ptr<ImGuiItem> m_imguiOverlay;
    std::vector<std::function<void(ImGuiContext *)>> m_imGuiOverlayDrawFunctions;

    KDGpu::RenderPassCommandRecorderOptions m_imguiPassOptions;
    KDGpu::Fence m_fence;
    KDGpu::CommandBuffer m_commandBuffer;
};

} // namespace KDGpuExample

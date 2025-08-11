/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "imgui_item.h"

#include <KDGpuExample/imgui_input_handler.h>
#include <KDGpuExample/imgui_renderer.h>

#include <KDGpu/device.h>

#include <imgui.h>

namespace KDGpuExample {

ImGuiItem::ImGuiItem(KDGpu::Device *device, KDGpu::Queue *queue)
{
    m_context = ImGui::CreateContext();
    m_input = std::make_unique<ImGuiInputHandler>();
    m_renderer = std::make_unique<ImGuiRenderer>(device, queue, m_context);
}

ImGuiItem::~ImGuiItem()
{
    ImGui::DestroyContext(m_context);
}

void ImGuiItem::initialize(float scaleFactor, KDGpu::SampleCountFlagBits samples, KDGpu::Format colorFormat, KDGpu::Format depthFormat)
{
    m_renderer->initialize(scaleFactor, samples, colorFormat, depthFormat);
}

void ImGuiItem::updateScale(const float scaleFactor)
{
    m_renderer->updateScale(scaleFactor);
}

void ImGuiItem::cleanup()
{
    m_renderer->cleanup();
}

void ImGuiItem::event(KDFoundation::EventReceiver *target, KDFoundation::Event *ev)
{
    ImGui::SetCurrentContext(m_context);

    m_input->event(target, ev);
}

void ImGuiItem::render(KDGpu::RenderPassCommandRecorder *recorder, const KDGpu::Extent2D &extent, uint32_t inFlightIndex, KDGpu::RenderPass *currentRenderPass, int lastSubpassIndex)
{
    // TODO: Should we split this out and call it as part of the updateScene() phase?
    // Update the geometry buffers
    if (m_renderer->updateGeometryBuffers(inFlightIndex))
        m_renderer->recordCommands(recorder, extent, inFlightIndex, currentRenderPass, lastSubpassIndex);
}

void ImGuiItem::renderDynamic(KDGpu::RenderPassCommandRecorder *recorder,
                              const KDGpu::Extent2D &extent,
                              uint32_t inFlightIndex)
{
    // Update the geometry buffers
    if (m_renderer->updateGeometryBuffers(inFlightIndex))
        m_renderer->recordCommands(recorder, extent, inFlightIndex, nullptr, 0, true);
}

} // namespace KDGpuExample

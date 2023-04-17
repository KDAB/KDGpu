#include "imgui_item.h"

#include <KDGpu_KDGui/imgui_input_handler.h>
#include <KDGpu_KDGui/imgui_renderer.h>

#include <KDGpu/device.h>

#include <imgui.h>

namespace KDGpuKDGui {

ImGuiItem::ImGuiItem(KDGpu::Device *device)
    : m_device(device)
{
    m_context = ImGui::CreateContext();
    m_input = std::make_unique<ImGuiInputHandler>();
    m_renderer = std::make_unique<ImGuiRenderer>(m_device, m_context);
}

ImGuiItem::~ImGuiItem()
{
    ImGui::DestroyContext(m_context);
}

void ImGuiItem::initialize(KDGpu::SampleCountFlagBits samples, KDGpu::Format colorFormat, KDGpu::Format depthFormat)
{
    m_renderer->initialize(samples, colorFormat, depthFormat);
}

void ImGuiItem::cleanup()
{
    m_renderer->cleanup();
}

void ImGuiItem::updateInputState()
{
    m_input->updateInputState();
}

void ImGuiItem::render(KDGpu::RenderPassCommandRecorder *recorder, const KDGpu::Extent2D &extent, uint32_t inFlightIndex)
{
    // TODO: Should we split this out and call it as part of the updateScene() phase?
    // Update the geometry buffers
    if (m_renderer->updateGeometryBuffers(inFlightIndex))
        m_renderer->recordCommands(recorder, extent, inFlightIndex);
}

} // namespace KDGpuKDGui

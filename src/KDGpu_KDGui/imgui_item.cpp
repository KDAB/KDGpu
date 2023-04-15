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

} // namespace KDGpuKDGui

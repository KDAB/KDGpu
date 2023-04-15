#pragma once

#include <KDGpu_KDGui/kdgpu_kdgui_export.h>

#include <memory>

struct ImGuiContext;

namespace KDGpu {
class Device;
}

namespace KDGpuKDGui {

class ImGuiInputHandler;
class ImGuiRenderer;

class KDGPU_KDGUI_EXPORT ImGuiItem
{
public:
    ImGuiItem(KDGpu::Device *device);
    ~ImGuiItem();

    ImGuiItem(const ImGuiItem &other) noexcept = delete;
    ImGuiItem &operator=(const ImGuiItem &other) noexcept = delete;

    ImGuiItem(ImGuiItem &&other) noexcept = default;
    ImGuiItem &operator=(ImGuiItem &&other) noexcept = default;

private:
    KDGpu::Device *m_device{ nullptr };
    ImGuiContext *m_context{ nullptr };
    std::unique_ptr<ImGuiInputHandler> m_input;
    std::unique_ptr<ImGuiRenderer> m_renderer;
};

} // namespace KDGpuKDGui

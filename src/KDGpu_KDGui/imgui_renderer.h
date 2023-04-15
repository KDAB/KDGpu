#pragma once

#include <KDGpu_KDGui/kdgpu_kdgui_export.h>

#include <KDGpu/shader_module.h>

namespace KDGpu {
class Device;
}

struct ImGuiContext;

namespace KDGpuKDGui {

class KDGPU_KDGUI_EXPORT ImGuiRenderer
{
public:
    ImGuiRenderer(KDGpu::Device *device, ImGuiContext *imGuiContext);
    ~ImGuiRenderer();

    ImGuiRenderer(const ImGuiRenderer &other) noexcept = delete;
    ImGuiRenderer &operator=(const ImGuiRenderer &other) noexcept = delete;

    ImGuiRenderer(ImGuiRenderer &&other) noexcept = default;
    ImGuiRenderer &operator=(ImGuiRenderer &&other) noexcept = default;

private:
    KDGpu::Device *m_device{ nullptr };
    ImGuiContext *m_imGuiContext{ nullptr };

    KDGpu::ShaderModule m_vertexShader;
    KDGpu::ShaderModule m_fragmentShader;
};

} // namespace KDGpuKDGui

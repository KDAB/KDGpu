#pragma once

#include <KDGpu_KDGui/kdgpu_kdgui_export.h>

#include <KDGpu/gpu_core.h>

#include <KDFoundation/object.h>

#include <memory>

struct ImGuiContext;

namespace KDGpu {
class Device;
struct Extent2D;
class Queue;
class RenderPassCommandRecorder;
} // namespace KDGpu

namespace KDGpuKDGui {

class ImGuiInputHandler;
class ImGuiRenderer;

class KDGPU_KDGUI_EXPORT ImGuiItem : public KDFoundation::Object
{
public:
    ImGuiItem(KDGpu::Device *device, KDGpu::Queue *queue);
    ~ImGuiItem();

    ImGuiItem(const ImGuiItem &other) noexcept = delete;
    ImGuiItem &operator=(const ImGuiItem &other) noexcept = delete;

    ImGuiItem(ImGuiItem &&other) noexcept = default;
    ImGuiItem &operator=(ImGuiItem &&other) noexcept = default;

    ImGuiContext *context() noexcept { return m_context; }

    void initialize(KDGpu::SampleCountFlagBits samples, KDGpu::Format colorFormat, KDGpu::Format depthFormat);
    void cleanup();

    void updateInputState();
    void event(KDFoundation::EventReceiver *target, KDFoundation::Event *ev);

    void render(KDGpu::RenderPassCommandRecorder *recorder, const KDGpu::Extent2D &extent, uint32_t inFlightIndex = 0);

private:
    ImGuiContext *m_context{ nullptr };
    std::unique_ptr<ImGuiInputHandler> m_input;
    std::unique_ptr<ImGuiRenderer> m_renderer;
};

} // namespace KDGpuKDGui

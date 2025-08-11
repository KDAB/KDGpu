/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpuExample/kdgpuexample_export.h>

#include <KDGpu/gpu_core.h>

#include <KDFoundation/object.h>

#include <memory>

struct ImGuiContext;

namespace KDGpu {
class Device;
struct Extent2D;
class Queue;
class RenderPassCommandRecorder;
class RenderPass;
} // namespace KDGpu

namespace KDGpuExample {

class ImGuiInputHandler;
class ImGuiRenderer;

/**
    @class ImGuiItem
    @brief ImGuiItem ...
    @ingroup kdgpuexample
    @headerfile imgui_item.h <KDGpuExample/imgui_item.h>
 */
class KDGPUEXAMPLE_EXPORT ImGuiItem : public KDFoundation::Object
{
public:
    ImGuiItem(KDGpu::Device *device, KDGpu::Queue *queue);
    ~ImGuiItem();

    ImGuiItem(const ImGuiItem &other) noexcept = delete;
    ImGuiItem &operator=(const ImGuiItem &other) noexcept = delete;

    ImGuiItem(ImGuiItem &&other) noexcept = default;
    ImGuiItem &operator=(ImGuiItem &&other) noexcept = default;

    ImGuiContext *context() noexcept { return m_context; }

    void initialize(float scaleFactor, KDGpu::SampleCountFlagBits samples, KDGpu::Format colorFormat, KDGpu::Format depthFormat);
    void updateScale(float scaleFactor);
    void cleanup();

    void event(KDFoundation::EventReceiver *target, KDFoundation::Event *ev);

    void render(KDGpu::RenderPassCommandRecorder *recorder,
                const KDGpu::Extent2D &extent,
                uint32_t inFlightIndex = 0,
                KDGpu::RenderPass *currentRenderPass = nullptr,
                int lastSubpassIndex = 0);

    void renderDynamic(KDGpu::RenderPassCommandRecorder *recorder,
                       const KDGpu::Extent2D &extent,
                       uint32_t inFlightIndex = 0);

private:
    ImGuiContext *m_context{ nullptr };
    std::unique_ptr<ImGuiInputHandler> m_input;
    std::unique_ptr<ImGuiRenderer> m_renderer;
};

} // namespace KDGpuExample

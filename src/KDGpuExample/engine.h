/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "engine_layer.h"

#include <KDGpuExample/kdgpuexample_export.h>

#include <KDFoundation/object.h>
#include <KDFoundation/logging.h>

#include <kdbindings/property.h>

#include <chrono>
#include <memory>
#include <vector>

using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

namespace KDGpuExample {

class KDGPUEXAMPLE_EXPORT Engine : public KDFoundation::Object
{
public:
    KDBindings::Property<bool> running{ false };
    KDBindings::Property<double> fps{ 0.0 };

    Engine();
    ~Engine() override;

    EngineLayer *attachEngineLayer(std::unique_ptr<EngineLayer> &&engineLayer);
    std::unique_ptr<EngineLayer> detachEngineLayer(EngineLayer *engineLayer);
    const std::vector<std::unique_ptr<EngineLayer>> &engineLayers() const { return m_engineLayers; }

    template<typename T, typename... Ts>
    T *createEngineLayer(Ts... args)
    {
        static_assert(std::is_base_of<EngineLayer, T>::value,
                      "The T type must inherit from EngineLayer.");
        auto engineLayer = std::make_unique<T>(std::forward<Ts>(args)...);
        return static_cast<T *>(this->attachEngineLayer(std::move(engineLayer)));
    }

    template<typename T>
    T *engineLayer() const
    {
        for (const auto &engineLayer : m_engineLayers) {
            const auto t = dynamic_cast<T *>(engineLayer.get());
            if (t)
                return t;
        }
        return nullptr;
    }

    void event(KDFoundation::EventReceiver *target, KDFoundation::Event *ev) override;

    void requestFrame();
    void doFrame();

    // Frame timing
    TimePoint startTime() const { return m_startTime; }
    TimePoint currentFrameTime() const { return m_currentFrameTime; }
    TimePoint previousFrameTime() const { return m_previousFrameTime; }
    uint64_t frameNumber() const { return m_totalFrameCounter; }
    float deltaTimeSeconds() const
    {
        return std::chrono::nanoseconds(
                       currentFrameTime() - previousFrameTime())
                       .count() *
                1.0e-9f;
    }

    std::chrono::nanoseconds simulationTime() const
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(m_currentFrameTime - m_startTime);
    }

private:
    void onRunningChanged(const bool &newValue);

    std::shared_ptr<spdlog::logger> m_logger;
    std::vector<std::unique_ptr<EngineLayer>> m_engineLayers;

    // Current and previous frame times
    TimePoint m_startTime;
    TimePoint m_currentFrameTime;
    TimePoint m_previousFrameTime;

    // Frames per second reporting
    uint32_t m_frameCounter{ 0 };
    uint64_t m_totalFrameCounter{ 0 };
    TimePoint m_lastFpsTimestamp;
};

} // namespace KDGpuExample

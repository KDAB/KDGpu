/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpuExample/example_engine_layer.h>
#include <KDGpuExample/kdgpuexample_export.h>

#include <array>

using namespace KDGpu;

namespace KDGpuExample {

class KDGPUEXAMPLE_EXPORT AdvancedExampleEngineLayer : public ExampleEngineLayer
{
public:
    AdvancedExampleEngineLayer();
    explicit AdvancedExampleEngineLayer(const SampleCountFlagBits samples);
    ~AdvancedExampleEngineLayer() override;

protected:
    void onAttached() override;
    void onDetached() override;
    void update() override;

    bool m_waitForPresentation{ true };
    std::array<Fence, MAX_FRAMES_IN_FLIGHT> m_frameFences;
};

} // namespace KDGpuExample

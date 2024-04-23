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

/**
    @class AdvancedExampleEngineLayer
    @brief AdvancedExampleEngineLayer ...
    @ingroup kdgpuexample
    @headerfile advanced_example_engine_layer.h <KDGpuExample/advanced_example_engine_layer.h>
 */
class KDGPUEXAMPLE_EXPORT AdvancedExampleEngineLayer : public ExampleEngineLayer
{
public:
    AdvancedExampleEngineLayer() = default;
    ~AdvancedExampleEngineLayer() override = default;

protected:
    void onAttached() override;
    void onDetached() override;
    void update() override;

    std::array<Fence, MAX_FRAMES_IN_FLIGHT> m_frameFences;
};

} // namespace KDGpuExample

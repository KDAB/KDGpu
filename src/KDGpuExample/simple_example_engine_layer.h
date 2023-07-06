/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpuExample/example_engine_layer.h>
#include <KDGpuExample/kdgpuexample_export.h>

using namespace KDGpu;

namespace KDGpuExample {

/**
    @class SimpleExampleEngineLayer
    @brief SimpleExampleEngineLayer ...
    @ingroup kdgpuexample
    @headerfile simple_example_engine_layer.h <KDGpuExample/simple_example_engine_layer.h>
 */
class KDGPUEXAMPLE_EXPORT SimpleExampleEngineLayer : public ExampleEngineLayer
{
public:
    SimpleExampleEngineLayer() = default;
    ~SimpleExampleEngineLayer() override;

protected:
    void update() override;
};

} // namespace KDGpuExample

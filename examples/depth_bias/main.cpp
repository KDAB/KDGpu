/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "depth_bias.h"

#include <KDGpuExample/engine.h>

#include <KDGui/gui_application.h>

//![0]
using namespace KDGui;
using namespace KDGpu;
using namespace KDGpuExample;

int main()
{
    GuiApplication app;
    app.applicationName = "Depth Bias";
    Engine engine;
    auto exampleLayer = engine.createEngineLayer<DepthBias>();
    engine.running = true;
    return app.exec();
}
//![0]

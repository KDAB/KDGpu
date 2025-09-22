/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "wireframe_geometry.h"

#include <KDGpuExample/engine.h>

#include <KDGui/gui_application.h>

//![0]
using namespace KDGui;
using namespace KDGpu;
using namespace KDGpuExample;

int main()
{
    GuiApplication app;
    app.applicationName = "Single-pass Wireframe Geometry Shader Example";
    Engine engine;
    auto exampleLayer = engine.createEngineLayer<WireframeGeometry>();
    engine.running = true;
    return app.exec();
}
//![0]

/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "hello_triangle.h"

#include <KDGpuExample/engine.h>

#include <KDGui/gui_application.h>

//![0]
using namespace KDGui;
using namespace KDGpu;
using namespace KDGpuExample;

int main()
{
    GuiApplication app;

#ifdef USE_HLSL_SHADERS
    app.applicationName = "Hello Triangle (HLSL)";
#elif defined(USE_SLANG_SHADERS)
    app.applicationName = "Hello Triangle (Slang)";
#else
    app.applicationName = "Hello Triangle (GLSL)";
#endif

    Engine engine;
    auto exampleLayer = engine.createEngineLayer<HelloTriangle>();
    engine.running = true;
    return app.exec();
}
//![0]

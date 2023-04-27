/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "textured_quad.h"

#include <KDGpuExample/engine.h>

#include <KDGui/gui_application.h>

using namespace KDGui;
using namespace KDGpu;
using namespace KDGpuExample;

int main()
{
    GuiApplication app;
    app.applicationName = "Textured Quad";
    Engine engine;
    auto exampleLayer = engine.createEngineLayer<TexturedQuad>();
    engine.running = true;
    return app.exec();
}

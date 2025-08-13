/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "render_to_texture_subpass_dynamic_rendering.h"

#include <KDGpuExample/engine.h>

#include <KDGui/gui_application.h>

using namespace KDGui;
using namespace KDGpu;
using namespace KDGpuExample;

int main()
{
    GuiApplication app;
    app.applicationName = "Render to Texture with 2 Subpasses using Dynamic Rendering";
    Engine engine;
    auto exampleLayer = engine.createEngineLayer<RenderToTextureSubpassDynamicRendering>();
    engine.running = true;
    return app.exec();
}

#include "textured_quad.h"

#include <KDGpu_KDGui/engine.h>

#include <KDGui/gui_application.h>

using namespace KDGui;
using namespace KDGpu;
using namespace KDGpuKDGui;

int main()
{
    GuiApplication app;
    Engine engine;
    auto exampleLayer = engine.createEngineLayer<TexturedQuad>();
    engine.running = true;
    return app.exec();
}

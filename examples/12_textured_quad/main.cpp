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

#include "hello_triangle.h"

#include <KDGpuExample/engine.h>

#include <KDGui/gui_application.h>

using namespace KDGui;
using namespace KDGpu;
using namespace KDGpuExample;

int main()
{
    GuiApplication app;
    app.applicationName = "Hello Triangle";
    Engine engine;
    auto exampleLayer = engine.createEngineLayer<HelloTriangle>();
    engine.running = true;
    return app.exec();
}

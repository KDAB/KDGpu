#include "rotating_triangle.h"

#include <KDGpu_KDGui/engine.h>

#include <KDGui/gui_application.h>

using namespace KDGui;
using namespace KDGpu;
using namespace KDGpuKDGui;

int main()
{
    GuiApplication app;
    app.applicationName = "Rotating Triangle";
    Engine engine;
    auto exampleLayer = engine.createEngineLayer<RotatingTriangle>();
    engine.running = true;
    return app.exec();
}

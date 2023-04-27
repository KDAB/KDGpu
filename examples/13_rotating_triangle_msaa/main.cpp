#include "rotating_triangle_msaa.h"

#include <KDGpuExample/engine.h>

#include <KDGui/gui_application.h>

using namespace KDGui;
using namespace KDGpu;
using namespace KDGpuExample;

int main()
{
    GuiApplication app;
    app.applicationName = "MSAA Rotating Triangle";
    Engine engine;
    auto exampleLayer = engine.createEngineLayer<RotatingTriangleMSAA>();
    engine.running = true;
    return app.exec();
}

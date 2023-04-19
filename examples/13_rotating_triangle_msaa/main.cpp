#include "rotating_triangle_msaa.h"

#include <KDGpu_KDGui/engine.h>

#include <KDGui/gui_application.h>

using namespace KDGui;
using namespace KDGpu;
using namespace KDGpuKDGui;

int main()
{
    GuiApplication app;
    app.applicationName = "MSAA Rotating Triangle";
    Engine engine;
    auto exampleLayer = engine.createEngineLayer<RotatingTriangleMSAA>();
    engine.running = true;
    return app.exec();
}

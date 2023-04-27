#include "dynamic_ubo_triangles.h"

#include <KDGpuExample/engine.h>

#include <KDGui/gui_application.h>

using namespace KDGui;
using namespace KDGpu;
using namespace KDGpuExample;

int main()
{
    GuiApplication app;
    app.applicationName = "Dynamic UBO";
    Engine engine;
    auto exampleLayer = engine.createEngineLayer<DynamicUBOTriangles>();
    engine.running = true;
    return app.exec();
}

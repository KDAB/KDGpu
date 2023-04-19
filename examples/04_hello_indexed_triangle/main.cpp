#include "hello_indexed_triangle.h"

#include <KDGpu_KDGui/engine.h>

#include <KDGui/gui_application.h>

using namespace KDGui;
using namespace KDGpu;
using namespace KDGpuKDGui;

int main()
{
    GuiApplication app;
    app.applicationName = "Hello Indexed Triangle";
    Engine engine;
    auto exampleLayer = engine.createEngineLayer<HelloIndexedTriangle>();
    engine.running = true;
    return app.exec();
}

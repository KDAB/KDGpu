#include "dynamic_ubo_triangles.h"

#include <kdgpu_kdgui/engine.h>

#include <KDGui/gui_application.h>

using namespace KDGui;
using namespace KDGpu;
using namespace KDGpuKDGui;

int main()
{
    GuiApplication app;
    Engine engine;
    auto exampleLayer = engine.createEngineLayer<DynamicUBOTriangles>();
    engine.running = true;
    return app.exec();
}

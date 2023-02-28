#include "hello_indexed_triangle.h"

#include <toy_renderer_kdgui/engine.h>

#include <KDGui/gui_application.h>

using namespace KDGui;
using namespace ToyRenderer;
using namespace ToyRendererKDGui;

int main()
{
    GuiApplication app;
    Engine engine;
    auto exampleLayer = engine.createEngineLayer<HelloIndexedTriangle>();
    engine.running = true;
    return app.exec();
}

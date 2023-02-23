#include "engine.h"
#include "hello_triangle.h"

#include <KDGui/gui_application.h>

using namespace KDGui;
using namespace ToyRenderer;

int main()
{
    GuiApplication app;
    Engine engine;
    auto exampleLayer = engine.createEngineLayer<HelloTriangle>();
    engine.running = true;
    return app.exec();
}

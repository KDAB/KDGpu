#include "engine.h"
#include "hello_triangle.h"

#include <Serenity/gui/gui_application.h>

using namespace Serenity;
using namespace ToyRenderer;

int main()
{
    GuiApplication app;
    Engine engine;
    auto exampleLayer = engine.createEngineLayer<HelloTriangle>();
    engine.running = true;
    return app.exec();
}

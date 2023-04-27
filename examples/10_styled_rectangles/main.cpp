#include "styled_rectangles.h"

#include <KDGpuExample/engine.h>

#include <KDGui/gui_application.h>

using namespace KDGui;
using namespace KDGpu;
using namespace KDGpuExample;

int main()
{
    GuiApplication app;
    app.applicationName = "Styled Rectangles";
    Engine engine;
    auto exampleLayer = engine.createEngineLayer<StyledRectangles>();
    engine.running = true;
    return app.exec();
}

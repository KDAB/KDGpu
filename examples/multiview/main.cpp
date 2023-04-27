#include "multiview.h"

#include <KDGpuExample/engine.h>

#include <KDGui/gui_application.h>

using namespace KDGui;
using namespace KDGpu;
using namespace KDGpuExample;

int main()
{
    GuiApplication app;
    app.applicationName = "MultiView";
    Engine engine;
    auto exampleLayer = engine.createEngineLayer<MultiView>();
    engine.running = true;
    return app.exec();
}

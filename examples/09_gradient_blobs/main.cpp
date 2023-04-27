#include "gradient_blobs.h"

#include <KDGpuExample/engine.h>

#include <KDGui/gui_application.h>

using namespace KDGui;
using namespace KDGpu;
using namespace KDGpuExample;

int main()
{
    GuiApplication app;
    app.applicationName = "Gradient Blobs";
    Engine engine;
    auto exampleLayer = engine.createEngineLayer<GradientBlobs>();
    engine.running = true;
    return app.exec();
}

#include "gradient_blobs.h"

#include <KDGpu_KDGui/engine.h>

#include <KDGui/gui_application.h>

using namespace KDGui;
using namespace KDGpu;
using namespace KDGpuKDGui;

int main()
{
    GuiApplication app;
    app.applicationName = "Gradient Blobs";
    Engine engine;
    auto exampleLayer = engine.createEngineLayer<GradientBlobs>();
    engine.running = true;
    return app.exec();
}

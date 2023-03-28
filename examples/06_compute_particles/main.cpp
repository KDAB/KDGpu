#include "compute_particles.h"

#include <KDGpu_KDGui/engine.h>

#include <KDGui/gui_application.h>

using namespace KDGui;
using namespace KDGpu;
using namespace KDGpuKDGui;

int main()
{
    GuiApplication app;
    Engine engine;
    auto exampleLayer = engine.createEngineLayer<ComputeParticles>();
    engine.running = true;
    return app.exec();
}

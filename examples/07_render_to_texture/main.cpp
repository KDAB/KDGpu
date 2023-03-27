#include "render_to_texture.h"

#include <kdgpu_kdgui/engine.h>

#include <KDGui/gui_application.h>

using namespace KDGui;
using namespace KDGpu;
using namespace KDGpuKDGui;

int main()
{
    GuiApplication app;
    Engine engine;
    auto exampleLayer = engine.createEngineLayer<RenderToTexture>();
    engine.running = true;
    return app.exec();
}

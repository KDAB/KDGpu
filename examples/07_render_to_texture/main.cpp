#include "render_to_texture.h"

#include <KDGpuExample/engine.h>

#include <KDGui/gui_application.h>

using namespace KDGui;
using namespace KDGpu;
using namespace KDGpuExample;

int main()
{
    GuiApplication app;
    app.applicationName = "Render to Texture";
    Engine engine;
    auto exampleLayer = engine.createEngineLayer<RenderToTexture>();
    engine.running = true;
    return app.exec();
}

#include "engine.h"
#include "example_engine_layer.h"

#include <toy_renderer/buffer_options.h>
#include <toy_renderer/device.h>
#include <toy_renderer/instance.h>
#include <toy_renderer/graphics_pipeline.h>
#include <toy_renderer/graphics_pipeline_options.h>
#include <toy_renderer/formatters.h>
#include <toy_renderer/gpu_core.h>
#include <toy_renderer/queue.h>
#include <toy_renderer/render_pass_options.h>
#include <toy_renderer/swapchain.h>
#include <toy_renderer/swapchain_options.h>
#include <toy_renderer/texture.h>
#include <toy_renderer/texture_options.h>
#include <toy_renderer/vulkan/vulkan_graphics_api.h>

#include <toy_renderer_serenity/view.h>

#include <Serenity/gui/gui_application.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <map>
#include <memory>
#include <span>
#include <vector>

using namespace Serenity;
using namespace ToyRenderer;

int main()
{
    GuiApplication app;
    Engine engine;
    auto exampleLayer = engine.createEngineLayer<ExampleEngineLayer>();

    // TODO: Put this stuff into an example-specific layer. Should we subclass or stack layers?

    // while (window.visible()) {
    //     // Acquire next swapchain image
    //     uint32_t currentImageIndex = 0;
    //     const auto result = swapchain.getNextImageIndex(currentImageIndex);
    //     if (result != true) {
    //         // Do we need to recreate the swapchain and dependent resources?
    //     }

    //     // Create a command encoder/recorder
    //     auto commandRecorder = device.createCommandRecorder();

    //     // Begin render pass
    //     opaquePassOptions.colorAttachments[0].view = swapchainViews.at(currentImageIndex).handle();
    //     auto opaquePass = commandRecorder.beginRenderPass(opaquePassOptions);

    //     // Bind pipeline
    //     opaquePass.setPipeline(pipeline.handle());

    //     // Bind vertex buffer
    //     opaquePass.setVertexBuffer(0, buffer.handle());

    //     // Bind any resources (none needed for hello_triangle)

    //     // Issue draw command
    //     const DrawCommand drawCmd = { .vertexCount = 3 };
    //     opaquePass.draw(drawCmd);

    //     // End render pass
    //     opaquePass.end();

    //     // End recording
    //     auto commands = commandRecorder.finish();

    //     // Submit command buffer to queue
    //     queue.submit(commands.handle());

    //     // Present and request next frame (need API for this)
    //     PresentOptions presentOptions = {
    //         .swapchainInfos = { { .swapchain = swapchain.handle(), .imageIndex = currentImageIndex } }
    //     };
    //     queue.present(presentOptions);
    // }

    return app.exec();
}

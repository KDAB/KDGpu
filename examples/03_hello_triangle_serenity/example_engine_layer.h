#pragma once

#include "engine_layer.h"

#include <toy_renderer_serenity/view.h>

#include <toy_renderer/device.h>
#include <toy_renderer/instance.h>
#include <toy_renderer/queue.h>
#include <toy_renderer/surface.h>
#include <toy_renderer/swapchain.h>
#include <toy_renderer/vulkan/vulkan_graphics_api.h>

#include <memory>

using namespace ToyRenderer;
using namespace ToyRendererSerenity;

class ExampleEngineLayer : public EngineLayer
{
public:
    ExampleEngineLayer();
    ~ExampleEngineLayer();

private:
    void onAttached() override;
    void onDetached() override;
    void update() override;

    std::unique_ptr<GraphicsApi> m_api;
    std::unique_ptr<View> m_window;

    Instance m_instance;
    Surface m_surface;
    Device m_device;
    Queue m_queue;
    Swapchain m_swapchain;
};

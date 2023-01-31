#pragma once

#include "engine_layer.h"

#include <toy_renderer_serenity/view.h>

#include <toy_renderer/device.h>
#include <toy_renderer/instance.h>
#include <toy_renderer/queue.h>
#include <toy_renderer/surface.h>
#include <toy_renderer/swapchain.h>
#include <toy_renderer/texture.h>
#include <toy_renderer/texture_view.h>
#include <toy_renderer/vulkan/vulkan_graphics_api.h>

#include <memory>
#include <vector>

using namespace ToyRenderer;
using namespace ToyRendererSerenity;

class ExampleEngineLayer : public EngineLayer
{
public:
    ExampleEngineLayer();
    ~ExampleEngineLayer();

protected:
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
    std::vector<TextureView> m_swapchainViews;
    Texture m_depthTexture;
    TextureView m_depthTextureView;

    const Format m_swapchainFormat{ Format::B8G8R8A8_UNORM };
    const Format m_depthFormat{ Format::D24_UNORM_S8_UINT };
};

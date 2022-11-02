#pragma once

#include <toy_renderer/api/api_instance.h>

#include <vulkan/vulkan.h>

namespace ToyRenderer {

class VulkanResourceManager;

struct VulkanInstance : public ApiInstance {
    explicit VulkanInstance(VulkanResourceManager *_vulkanResourceManager, VkInstance _instance);

    std::vector<Handle<Adapter_t>> queryAdapters() final;

#if defined(TOY_RENDERER_PLATFORM_WIN32)
    Handle<Surface_t> createSurface(HWND hWnd) final;
#endif
#if defined(TOY_RENDERER_PLATFORM_LINUX)
    Handle<Surface_t> createSurface(xcb_connection_t *connection, xcb_window_t window) final;
#endif
#if defined(TOY_RENDERER_PLATFORM_MACOS)
    Handle<Surface_t> createSurface(CAMetalLayer *layer) final;
#endif
#if defined(TOY_RENDERER_PLATFORM_SERENITY)
    Handle<Surface_t> createSurface(Serenity::Window *window) final;
#endif

    VulkanResourceManager *vulkanResourceManager{ nullptr };
    VkInstance instance{ VK_NULL_HANDLE };
};

} // namespace ToyRenderer

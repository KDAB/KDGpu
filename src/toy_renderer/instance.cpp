#include "instance.h"

#include <toy_renderer/graphics_api.h>
#include <toy_renderer/resource_manager.h>
#include <toy_renderer/api/api_instance.h>

namespace ToyRenderer {

Instance::Instance(GraphicsApi *api, const InstanceOptions &options)
{
    // Create an instance using the underlying API
    m_api = api;
    m_instance = m_api->resourceManager()->createInstance(options);
}

Instance::~Instance()
{
    // TODO: Destroy the instance using the underlying API
}

std::span<Adapter> Instance::adapters()
{
    if (m_adapters.empty()) {
        auto apiInstance = m_api->resourceManager()->getInstance(m_instance);
        auto adapterHandles = apiInstance->queryAdapters();
        const auto adapterCount = static_cast<uint32_t>(adapterHandles.size());
        m_adapters.reserve(adapterCount);
        for (uint32_t adapterIndex = 0; adapterIndex < adapterCount; ++adapterIndex)
            m_adapters.emplace_back(Adapter{ m_api, adapterHandles[adapterIndex] });
    }
    return std::span{ m_adapters };
}

#if defined(TOY_RENDERER_PLATFORM_WIN32)
Handle<Surface_t> Instance::createSurface(HINSTANCE hInstance, HWND hWnd)
{
    return {};
}
#endif

#if defined(TOY_RENDERER_PLATFORM_LINUX)
Handle<Surface_t> Instance::createSurface(xcb_connection_t *connection, xcb_window_t window);
#endif

#if defined(TOY_RENDERER_PLATFORM_MACOS)
Handle<Surface_t> Instance::createSurface(CAMetalLayer *layer);
#endif

#if defined(TOY_RENDERER_PLATFORM_SERENITY)
Handle<Surface_t> Instance::createSurface(Serenity::Window *window)
{
    return {};
}
#endif

} // namespace ToyRenderer

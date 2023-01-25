#pragma once

#include <toy_renderer/adapter.h>
#include <toy_renderer/device.h>
#include <toy_renderer/gpu_core.h>
#include <toy_renderer/handle.h>
#include <toy_renderer/surface.h>
#include <toy_renderer/surface_options.h>

#include <toy_renderer/toy_renderer_export.h>

#include <span>
#include <string>
#include <vector>

#if defined(TOY_RENDERER_PLATFORM_WIN32)
#define NOMINMAX
#include <windows.h>
#endif
#if defined(TOY_RENDERER_PLATFORM_LINUX)

#endif
#if defined(TOY_RENDERER_PLATFORM_MACOS)

#endif
#if defined(TOY_RENDERER_PLATFORM_SERENITY)
#include <Serenity/gui/window.h>
#endif

namespace ToyRenderer {

class GraphicsApi;
class Surface;
struct Instance_t;

struct InstanceOptions {
    std::string applicationName{ "Serenity Application" };
    uint32_t applicationVersion{ SERENITY_MAKE_API_VERSION(0, 1, 0, 0) };
    std::vector<std::string> layers;
    std::vector<std::string> extensions;
};

struct AdapterAndDevice {
    Adapter adapter;
    Device device;
};

class TOY_RENDERER_EXPORT Instance
{
public:
    ~Instance();

    bool isValid() const { return m_instance.isValid(); }

    AdapterAndDevice createDefaultDevice(const Surface &surface);

    std::span<Adapter> adapters();

    // TODO: Support Serenity::Window, QWindow etc
    //
    // We could provide a tiny library that links to both Serenity and ToyRenderer
    // and exposed SerenityVulkanGraphicsApi which creates a SerenityInstance class that inherits
    // Instance and which creates a Surface from a Serenity::Window. This approach would keep
    // ToyRenderer separate from Serenity/Qt.
    Surface createSurface(const SurfaceOptions &options);

private:
    Instance(GraphicsApi *api, const InstanceOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Instance_t> m_instance;
    std::vector<Adapter> m_adapters;

    friend class GraphicsApi;
};

} // namespace ToyRenderer

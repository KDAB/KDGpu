#pragma once

#include <toy_renderer/adapter.h>
#include <toy_renderer/gpu_core.h>
#include <toy_renderer/handle.h>

#include <toy_renderer/toy_renderer_export.h>

#include <span>
#include <string>
#include <vector>

namespace ToyRenderer {

class GraphicsApi;
struct Instance_t;

struct InstanceOptions {
    std::string applicationName{ "Serenity Application" };
    uint32_t applicationVersion{ SERENITY_MAKE_API_VERSION(0, 1, 0, 0) };
    std::vector<std::string> layers;
    std::vector<std::string> extensions;
};

class TOY_RENDERER_EXPORT Instance
{
public:
    ~Instance();

    bool isValid() const { return m_instance.isValid(); }

    std::span<Adapter> adapters();

private:
    Instance(GraphicsApi *api, const InstanceOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Instance_t> m_instance;
    std::vector<Adapter> m_adapters;

    friend class GraphicsApi;
};

} // namespace ToyRenderer

#pragma once

#include <toy_renderer/adapter_features.h>
#include <toy_renderer/adapter_properties.h>
#include <toy_renderer/handle.h>

#include <toy_renderer/toy_renderer_export.h>

#include <string>
#include <vector>

namespace ToyRenderer {

class GraphicsApi;

struct Adapter_t;
struct Instance_t;

struct AdapterOptions {
    std::vector<std::string> layers;
    std::vector<std::string> extensions;
};

class TOY_RENDERER_EXPORT Adapter
{
public:
    ~Adapter();

    bool isValid() const { return m_adapter.isValid(); }

private:
    explicit Adapter(GraphicsApi *api, const Handle<Adapter_t> &adapter);

    GraphicsApi *m_api{ nullptr };
    Handle<Adapter_t> m_adapter;

    friend class Instance;
};

} // namespace ToyRenderer

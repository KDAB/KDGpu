#pragma once

#include <toy_renderer/handle.h>

#include <toy_renderer/toy_renderer_export.h>

#include <string>
#include <vector>

namespace ToyRenderer {

class GraphicsApi;
struct Device_t;

struct DeviceOptions {
    std::vector<std::string> layers;
    std::vector<std::string> extensions;
};

class TOY_RENDERER_EXPORT Device
{
public:
    ~Device();

private:
    Device(GraphicsApi *api, const DeviceOptions &options);

    GraphicsApi *m_api{ nullptr };

    friend class Adapter;
};

} // namespace ToyRenderer

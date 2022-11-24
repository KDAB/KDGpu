#pragma once

#include <toy_renderer/handle.h>

namespace ToyRenderer {

class GraphicsApi;

struct Device_t;
struct ShaderModule_t;

class ShaderModule
{
public:
    ~ShaderModule();

    Handle<ShaderModule_t> handle() const noexcept { return m_shaderModule; }
    bool isValid() const noexcept { return m_shaderModule.isValid(); }

private:
    ShaderModule(GraphicsApi *api, const Handle<Device_t> &device, const Handle<ShaderModule_t> &shaderModule);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<ShaderModule_t> m_shaderModule;

    friend class Device;
};

} // namespace ToyRenderer

#pragma once

#include <toy_renderer/handle.h>

#include <toy_renderer/toy_renderer_export.h>

namespace ToyRenderer {

class GraphicsApi;

struct Device_t;
struct ShaderModule_t;

class TOY_RENDERER_EXPORT ShaderModule
{
public:
    ~ShaderModule();

    Handle<ShaderModule_t> handle() const noexcept { return m_shaderModule; }
    bool isValid() const noexcept { return m_shaderModule.isValid(); }

    operator Handle<ShaderModule_t>() const noexcept { return m_shaderModule; }

private:
    ShaderModule(GraphicsApi *api, const Handle<Device_t> &device, const Handle<ShaderModule_t> &shaderModule);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<ShaderModule_t> m_shaderModule;

    friend class Device;
};

} // namespace ToyRenderer

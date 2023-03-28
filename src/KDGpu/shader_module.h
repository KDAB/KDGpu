#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>
#include <string>

namespace KDGpu {

class GraphicsApi;

struct Device_t;
struct ShaderModule_t;

class KDGPU_EXPORT ShaderModule
{
public:
    ShaderModule();
    ~ShaderModule();

    ShaderModule(ShaderModule &&);
    ShaderModule &operator=(ShaderModule &&);

    ShaderModule(const ShaderModule &) = delete;
    ShaderModule &operator=(const ShaderModule &) = delete;

    Handle<ShaderModule_t> handle() const noexcept { return m_shaderModule; }
    bool isValid() const noexcept { return m_shaderModule.isValid(); }

    operator Handle<ShaderModule_t>() const noexcept { return m_shaderModule; }

private:
    ShaderModule(GraphicsApi *api, const Handle<Device_t> &device, const std::vector<uint32_t> &code);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<ShaderModule_t> m_shaderModule;

    friend class Device;
};

KDGPU_EXPORT std::vector<uint32_t> readShaderFile(const std::string &filename);

} // namespace KDGpu

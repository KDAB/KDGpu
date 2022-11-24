#include "shader_module.h"

namespace ToyRenderer {

ShaderModule::ShaderModule(GraphicsApi *api, const Handle<Device_t> &device, const Handle<ShaderModule_t> &shaderModule)
    : m_api(api)
    , m_device(device)
    , m_shaderModule(shaderModule)
{
}

ShaderModule::~ShaderModule()
{
}

} // namespace ToyRenderer

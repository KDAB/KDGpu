#include "shader_module.h"
#include <fstream>

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

std::vector<uint32_t> readShaderFile(const std::string &filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Failed to open file");

    const size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<uint32_t> buffer(fileSize / 4);
    file.seekg(0);
    file.read(reinterpret_cast<char *>(buffer.data()), static_cast<std::streamsize>(fileSize));
    file.close();
    return buffer;
}

} // namespace ToyRenderer

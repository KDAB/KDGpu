/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "shader_module.h"
#include <fstream>

#include <KDGpu/graphics_api.h>
#include <KDGpu/resource_manager.h>
#include <KDGpu/api/api_shader_module.h>

namespace KDGpu {

ShaderModule::ShaderModule() = default;

ShaderModule::ShaderModule(GraphicsApi *api, const Handle<Device_t> &device, const std::vector<uint32_t> &code)
    : m_api(api)
    , m_device(device)
    , m_shaderModule(m_api->resourceManager()->createShaderModule(m_device, code))
{
}

ShaderModule::~ShaderModule()
{
    if (isValid())
        m_api->resourceManager()->deleteShaderModule(handle());
}

ShaderModule::ShaderModule(ShaderModule &&other)
{
    m_api = other.m_api;
    m_device = other.m_device;
    m_shaderModule = other.m_shaderModule;

    other.m_api = nullptr;
    other.m_device = {};
    other.m_shaderModule = {};
}

ShaderModule &ShaderModule::operator=(ShaderModule &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteShaderModule(handle());

        m_api = other.m_api;
        m_device = other.m_device;
        m_shaderModule = other.m_shaderModule;

        other.m_api = nullptr;
        other.m_device = {};
        other.m_shaderModule = {};
    }
    return *this;
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

} // namespace KDGpu

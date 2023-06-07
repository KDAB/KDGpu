/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "shader_module.h"
#include <fstream>

#include <KDUtils/file.h>
#include <KDUtils/dir.h>
#include <KDGpu/graphics_api.h>
#include <KDGpu/resource_manager.h>
#include <KDGpu/api/api_shader_module.h>

#include <KDGpu/utils/logging.h>

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
    using namespace KDUtils;

    File file(File::exists(filename) ? filename : Dir::applicationDir().absoluteFilePath(filename));

    if (!file.open(std::ios::in | std::ios::binary)) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to open file {}", filename);
        throw std::runtime_error("Failed to open file");
    }

    const ByteArray fileContent = file.readAll();
    std::vector<uint32_t> buffer(fileContent.size() / 4);
    std::memcpy(buffer.data(), fileContent.data(), fileContent.size());

    return buffer;
}

} // namespace KDGpu

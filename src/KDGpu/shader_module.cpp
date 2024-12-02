/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "shader_module.h"

#include <KDGpu/api/graphics_api_impl.h>

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

ShaderModule::ShaderModule(ShaderModule &&other) noexcept
{
    m_api = std::exchange(other.m_api, nullptr);
    m_device = std::exchange(other.m_device, {});
    m_shaderModule = std::exchange(other.m_shaderModule, {});
}

ShaderModule &ShaderModule::operator=(ShaderModule &&other) noexcept
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteShaderModule(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_device = std::exchange(other.m_device, {});
        m_shaderModule = std::exchange(other.m_shaderModule, {});
    }
    return *this;
}

} // namespace KDGpu

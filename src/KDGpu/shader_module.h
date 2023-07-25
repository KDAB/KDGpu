/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>
#include <string>

namespace KDGpu {

class GraphicsApi;

struct Device_t;
struct ShaderModule_t;

/**
 * @brief ShaderModule
 * @ingroup public
 */
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

} // namespace KDGpu

/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/instance.h>
#include <KDGpu/resource_manager.h>
#include <KDGpu/kdgpu_export.h>

namespace KDGpu {

/**
 * @defgroup api API
 *
 * Holds the interfaces for the Rendering APIs
 */

/**
    @brief GraphicsApi is the link between our Application and the Rendering API
    @ingroup api
    @ingroup public
    @headerfile graphics_api.h <KDGpu/graphics_api.h>

    @sa KDGpu::VulkanGraphicsAPI
 */
class KDGPU_EXPORT GraphicsApi
{
public:
    virtual ~GraphicsApi();

    enum class Api : uint8_t {
        Vulkan = 0,
        UserDefined = 255
    };
    Api api() const noexcept { return m_api; }
    virtual const char *apiName() const noexcept = 0;

    /**
     * @brief Create an Instance object given the InstanceOptions @a options
     */
    Instance createInstance(const InstanceOptions &options = InstanceOptions());

    /**
     * @brief Returns the ResourceManager instance for the GraphicsApi
     */
    ResourceManager *resourceManager() noexcept { return m_resourceManager; }
    const ResourceManager *resourceManager() const noexcept { return m_resourceManager; }

protected:
    explicit GraphicsApi(Api api);

    ResourceManager *m_resourceManager{ nullptr };
    Api m_api{ Api::UserDefined };
};

} // namespace KDGpu

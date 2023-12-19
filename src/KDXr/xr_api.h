/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/instance.h>
#include <KDXr/resource_manager.h>
#include <KDXr/kdxr_export.h>

namespace KDXr {

/**
 * @defgroup api API
 *
 * Holds the interfaces for the Rendering APIs
 */

/**
    @brief XrApi is the link between our Application and the XR API
    @ingroup api
    @ingroup public
    @headerfile xr_api.h <KDGpu/xr_api.h>

    @sa KDXr::OpenXrAPI
 */
class KDXR_EXPORT XrApi
{
public:
    virtual ~XrApi();

    /**
     * @brief Create an Instance object given the InstanceOptions @a options
     */
    Instance createInstance(const InstanceOptions &options = InstanceOptions());

    /**
     * @brief Returns the ResourceManager instance for the XrApi
     */
    ResourceManager *resourceManager() noexcept { return m_resourceManager; }
    const ResourceManager *resourceManager() const noexcept { return m_resourceManager; }

protected:
    XrApi();

    ResourceManager *m_resourceManager{ nullptr };
};

} // namespace KDXr

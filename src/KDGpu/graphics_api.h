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
 * @brief GraphicsApi
 * \ingroup api
 * \ingroup public
 *
 */
class KDGPU_EXPORT GraphicsApi
{
public:
    virtual ~GraphicsApi();

    Instance createInstance(const InstanceOptions &options = InstanceOptions());

    ResourceManager *resourceManager() noexcept { return m_resourceManager; }
    const ResourceManager *resourceManager() const noexcept { return m_resourceManager; }

protected:
    GraphicsApi();

    ResourceManager *m_resourceManager{ nullptr };
};

} // namespace KDGpu

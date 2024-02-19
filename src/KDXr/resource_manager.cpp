/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "resource_manager.h"

namespace KDXr {

/**
    @class ResourceManager
    @brief ResourceManager manages XR API resources
    @ingroup api
    @ingroup public
    @headerfile resource_manager.h <KDXr/resource_manager.h>
*/

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
}

std::vector<ApiLayer> ResourceManager::availableApiLayers() const
{
    return {};
}

std::vector<Extension> ResourceManager::availableInstanceExtensions() const
{
    return {};
}

} // namespace KDXr

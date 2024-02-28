/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "xr_api.h"

namespace KDXr {

XrApi::XrApi()
{
}

XrApi::~XrApi()
{
}

std::vector<ApiLayer> XrApi::availableApiLayers() const
{
    return m_resourceManager->availableApiLayers();
}

std::vector<Extension> XrApi::availableInstanceExtensions() const
{
    return m_resourceManager->availableInstanceExtensions();
}

Instance XrApi::createInstance(const InstanceOptions &options)
{
    return Instance(this, options);
}

} // namespace KDXr

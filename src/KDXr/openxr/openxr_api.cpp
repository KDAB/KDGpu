/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "openxr_api.h"

namespace KDXr {

OpenXrApi::OpenXrApi()
    : XrApi()
    , m_openxrResourceManager{ std::make_unique<OpenXrResourceManager>() }
{
    m_resourceManager = m_openxrResourceManager.get();
}

OpenXrApi::~OpenXrApi()
{
}

} // namespace KDXr

/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/xr_api.h>
#include <KDXr/kdxr_export.h>
#include <KDXr/openxr/openxr_resource_manager.h>

#include <memory>

namespace KDXr {

/**
 * @defgroup openxr OpenXR
 *
 * Holds the OpenXR implementation of the XR API Interfaces
 */

/**
 * @page OpenXR
 *
 */

/**
 * @brief OpenXRApi
 * \ingroup openxr
 * \ingroup public
 *
 */
class KDXR_EXPORT OpenXrApi final : public XrApi
{
public:
    OpenXrApi();
    ~OpenXrApi() final;

private:
    std::unique_ptr<OpenXrResourceManager> m_openxrResourceManager;
};

} // namespace KDXr

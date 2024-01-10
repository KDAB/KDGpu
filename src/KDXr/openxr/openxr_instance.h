/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/api/api_instance.h>
#include <KDXr/kdxr_export.h>
#include <KDXr/config.h>

#include <openxr/openxr.h>

namespace KDXr {

class OpenXrResourceManager;

/**
 * @brief OpenXrInstance
 * \ingroup openxr
 *
 */
struct KDXR_EXPORT OpenXrInstance : public ApiInstance {
    explicit OpenXrInstance(OpenXrResourceManager *_openxrResourceManager,
                            XrInstance _instance,
                            std::vector<ApiLayer> &_apiLayers,
                            std::vector<Extension> &_extensions,
                            bool _isOwned = true) noexcept;

    std::vector<ApiLayer> enabledApiLayers() const final;
    std::vector<Extension> enabledExtensions() const final;

    OpenXrResourceManager *openxrResourceManager{ nullptr };
    XrInstance instance{ XR_NULL_HANDLE };
    XrDebugUtilsMessengerEXT debugMessenger{ nullptr };
    bool isOwned{ true };
    std::vector<ApiLayer> apiLayers;
    std::vector<Extension> extensions;
};

} // namespace KDXr

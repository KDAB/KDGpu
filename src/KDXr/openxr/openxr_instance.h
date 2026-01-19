/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/kdxr_export.h>
#include <KDXr/config.h>
#include <KDXr/kdxr_core.h>

#include <KDGpu/handle.h>

#include <openxr/openxr.h>

#include <map>

namespace KDXr {

struct Session_t;
class OpenXrResourceManager;
struct Instance_t;
struct System_t;
struct SuggestActionBindingsOptions;
struct SystemOptions;
class Instance;

/**
 * @brief OpenXrInstance
 * \ingroup openxr
 *
 */
struct KDXR_EXPORT OpenXrInstance {
    explicit OpenXrInstance(OpenXrResourceManager *_openxrResourceManager,
                            XrInstance _instance,
                            std::vector<ApiLayer> &_apiLayers,
                            std::vector<Extension> &_extensions,
                            bool _isOwned = true) noexcept;

    void initialize(Instance *_frontendInstance);
    InstanceProperties properties() const;
    std::vector<ApiLayer> enabledApiLayers() const;
    std::vector<Extension> enabledExtensions() const;
    KDGpu::Handle<System_t> querySystem(const SystemOptions &options, const KDGpu::Handle<Instance_t> &instanceHandle);
    virtual ProcessEventsResult processEvents();
    void processSessionStateChangedEvent(const XrEventDataSessionStateChanged *eventData);
    SuggestActionBindingsResult suggestActionBindings(const SuggestActionBindingsOptions &options);

    XrPath createXrPath(const std::string &path) const;
    std::string pathToString(XrPath path) const;

    OpenXrResourceManager *openxrResourceManager{ nullptr };
    XrInstance instance{ XR_NULL_HANDLE };
    XrDebugUtilsMessengerEXT debugMessenger{ XR_NULL_HANDLE };
    bool isOwned{ true };
    std::vector<ApiLayer> apiLayers;
    std::vector<Extension> extensions;
    KDGpu::Handle<System_t> systemHandle;

    Instance *frontendInstance{ nullptr };
    std::map<XrSession, KDGpu::Handle<Session_t>> m_sessionToHandle;
};

} // namespace KDXr

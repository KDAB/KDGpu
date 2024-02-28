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

#include <KDGpu/handle.h>

#include <openxr/openxr.h>

#include <map>

namespace KDXr {

struct Session_t;
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

    void initialize(Instance *_frontendInstance) final;
    InstanceProperties properties() const final;
    std::vector<ApiLayer> enabledApiLayers() const final;
    std::vector<Extension> enabledExtensions() const final;
    KDGpu::Handle<System_t> querySystem(const SystemOptions &options, const KDGpu::Handle<Instance_t> &instanceHandle) final;
    virtual ProcessEventsResult processEvents() final;
    void processSessionStateChangedEvent(const XrEventDataSessionStateChanged *eventData);
    SuggestActionBindingsResult suggestActionBindings(const SuggestActionBindingsOptions &options) final;

    XrPath createXrPath(const std::string &path) const;
    std::string pathToString(XrPath path) const;

    OpenXrResourceManager *openxrResourceManager{ nullptr };
    XrInstance instance{ XR_NULL_HANDLE };
    XrDebugUtilsMessengerEXT debugMessenger{ nullptr };
    bool isOwned{ true };
    std::vector<ApiLayer> apiLayers;
    std::vector<Extension> extensions;
    KDGpu::Handle<System_t> systemHandle;

    Instance *frontendInstance{ nullptr };
    std::map<XrSession, KDGpu::Handle<Session_t>> m_sessionToHandle;
};

} // namespace KDXr

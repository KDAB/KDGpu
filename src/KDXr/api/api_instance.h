/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/kdxr_core.h>
#include <KDXr/handle.h>

#include <vector>

namespace KDXr {

struct Instance_t;
struct System_t;
struct SuggestActionBindingsOptions;
struct SystemOptions;
class Instance;

/**
 * @brief ApiInstance
 * \ingroup api
 *
 */
struct ApiInstance {
    virtual void initialize(Instance *frontendInstance) = 0;
    virtual InstanceProperties properties() const = 0;
    virtual std::vector<ApiLayer> enabledApiLayers() const = 0;
    virtual std::vector<Extension> enabledExtensions() const = 0;
    virtual Handle<System_t> querySystem(const SystemOptions &options, const Handle<Instance_t> &instanceHandle) = 0;
    virtual ProcessEventsResult processEvents() = 0;
    virtual SuggestActionBindingsResult suggestActionBindings(const SuggestActionBindingsOptions &options) = 0;
};

} // namespace KDXr

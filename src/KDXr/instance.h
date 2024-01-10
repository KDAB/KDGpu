/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/kdxr_core.h>
#include <KDXr/handle.h>
#include <KDXr/kdxr_export.h>

#include <span>
#include <string>
#include <vector>

namespace KDXr {

struct Instance_t;
class XrApi;

/**
    @brief Holds option fields used for Instance creation
    @ingroup public
    @headerfile instance.h <KDXr/instance.h>
 */
struct InstanceOptions {
    std::string applicationName{ "KDXr Application" };
    uint32_t applicationVersion{ KDXR_MAKE_API_VERSION(0, 1, 0, 0) };
    std::vector<std::string> layers;
    std::vector<std::string> extensions;
};

class KDXR_EXPORT Instance
{
public:
    Instance();
    ~Instance();

    Instance(Instance &&);
    Instance &operator=(Instance &&);

    Instance(const Instance &) = delete;
    Instance &operator=(const Instance &) = delete;

    Handle<Instance_t> handle() const noexcept { return m_instance; }
    bool isValid() const { return m_instance.isValid(); }

    operator Handle<Instance_t>() const noexcept { return m_instance; }

    std::vector<ApiLayer> enabledApiLayers() const;
    std::vector<Extension> enabledExtensions() const;

private:
    Instance(XrApi *api, const InstanceOptions &options);

    XrApi *m_api{ nullptr };
    Handle<Instance_t> m_instance;

    friend class XrApi;
    friend class OpenXrGraphicsApi;
};

} // namespace KDXr

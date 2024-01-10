/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <stdint.h>
#include <string>

using HANDLE = void *;

#define KDXR_MAKE_API_VERSION(variant, major, minor, patch) \
    ((((uint32_t)(variant)) << 29) | (((uint32_t)(major)) << 22) | (((uint32_t)(minor)) << 12) | ((uint32_t)(patch)))

#define KDXR_VERSION_MAJOR(version) (uint16_t)(((uint64_t)(version) >> 48) & 0xffffULL)
#define KDXR_VERSION_MINOR(version) (uint16_t)(((uint64_t)(version) >> 32) & 0xffffULL)
#define KDXR_VERSION_PATCH(version) (uint32_t)((uint64_t)(version)&0xffffffffULL)

namespace KDXr {

/**
 * @defgroup public Public API
 *
 * Holds the Public API
 */

/*! \addtogroup public
 *  @{
 */

struct ApiLayer {
    std::string name;
    std::string description;
    uint64_t specVersion{ 0 };
    uint32_t layerVersion{ 0 };

    friend bool operator==(const ApiLayer &, const ApiLayer &) = default;
};

struct Extension {
    std::string name;
    uint32_t extensionVersion{ 0 };

    friend bool operator==(const Extension &, const Extension &) = default;
};

struct InstanceProperties {
    std::string runtimeName;
    uint64_t runtimeVersion{ 0 };
};

struct SystemGraphicsProperties {
    uint32_t maxSwapchainWidth{ 0 };
    uint32_t maxSwapchainHeight{ 0 };
    uint32_t maxLayerCount{ 0 };
};

struct SystemTrackingProperties {
    bool hasOrientationTracking{ false };
    bool hasPositionTracking{ false };
};

struct SystemProperties {
    uint32_t vendorId{ 0 };
    std::string systemName;
    SystemGraphicsProperties graphicsProperties{};
    SystemTrackingProperties trackingProperties{};
};

enum class FormFactor : uint32_t {
    HeadMountedDisplay = 1,
    HandheldDisplay = 2,
    MaxEnum = 0x7fffffff
};

} // namespace KDXr

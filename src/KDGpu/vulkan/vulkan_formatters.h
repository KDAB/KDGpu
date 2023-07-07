/*
    This file is part of KDGpu.

    SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

    SPDX-License-Identifier: MIT

    Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/utils/logging.h>

#include <vulkan/vulkan.h>

inline std::string getResultAsString(VkResult vulkan_result)
{
    switch (vulkan_result) {
    case VK_SUCCESS:
        return "SUCCESS";
    case VK_ERROR_OUT_OF_HOST_MEMORY:
        return "OUT OF HOST MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        return "OUT OF DEVICE MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED:
        return "INITIALIZATION FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT:
        return "LAYER NOT PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT:
        return "EXTENSION NOT PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER:
        return "INCOMPATIBLE DRIVER";
    default:
        return "UNKNOWN RESULT";
    }
}

template<>
struct KDGPU_EXPORT fmt::formatter<VkResult> : fmt::formatter<std::string> {
    template<typename FormatContext>
    auto format(VkResult const &result, FormatContext &ctx)
    {
        return fmt::formatter<std::string>::format(getResultAsString(result), ctx);
    }
};

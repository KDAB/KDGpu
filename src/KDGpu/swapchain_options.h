/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>

#include <vector>

namespace KDGpu {

struct Swapchain_t;
struct Surface_t;

struct SwapchainOptions {
    std::string_view label;
    Handle<Surface_t> surface;
    Format format{ Format::B8G8R8A8_UNORM };
    ColorSpace colorSpace{ ColorSpace::SRgbNonlinear };
    uint32_t minImageCount{ 3 };
    Extent2D imageExtent;
    uint32_t imageLayers{ 1 };
    TextureUsageFlags imageUsageFlags{ TextureUsageFlagBits::ColorAttachmentBit };
    SharingMode imageSharingMode{ SharingMode::Exclusive };
    std::vector<uint32_t> queueTypeIndices;
    SurfaceTransformFlagBits transform{ SurfaceTransformFlagBits::IdentityBit };
    CompositeAlphaFlagBits compositeAlpha{ CompositeAlphaFlagBits::OpaqueBit };
    PresentMode presentMode{ PresentMode::Mailbox };
    bool clipped{ true };
    Handle<Swapchain_t> oldSwapchain; // Optional, can be useful when recreating a swapchain
};

} // namespace KDGpu

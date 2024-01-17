/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "openxr_enums.h"

namespace KDXr {

XrFormFactor formFactorToXrFormFactor(FormFactor formFactor)
{
    return static_cast<XrFormFactor>(formFactor);
}

XrSwapchainUsageFlags swapchainUsageFlagsToXrSwapchainUsageFlags(SwapchainUsageFlags flags)
{
    return static_cast<XrSwapchainUsageFlags>(flags.toInt());
}

KDGpu::TextureUsageFlags kdxrSwapchainUsageFlagsToKDGpuTextureUsageFlags(SwapchainUsageFlags flags)
{
    KDGpu::TextureUsageFlags result;

    if (flags.testFlag(SwapchainUsageFlagBits::ColorAttachmentBit))
        result.setFlag(KDGpu::TextureUsageFlagBits::ColorAttachmentBit);

    if (flags.testFlag(SwapchainUsageFlagBits::DepthStencilAttachmentBit))
        result.setFlag(KDGpu::TextureUsageFlagBits::DepthStencilAttachmentBit);

    if (flags.testFlag(SwapchainUsageFlagBits::UnorderedAccessBit))
        result.setFlag(KDGpu::TextureUsageFlagBits::StorageBit);

    if (flags.testFlag(SwapchainUsageFlagBits::TransferSrcBit))
        result.setFlag(KDGpu::TextureUsageFlagBits::TransferSrcBit);

    if (flags.testFlag(SwapchainUsageFlagBits::TransferDstBit))
        result.setFlag(KDGpu::TextureUsageFlagBits::TransferDstBit);

    if (flags.testFlag(SwapchainUsageFlagBits::SampledBit))
        result.setFlag(KDGpu::TextureUsageFlagBits::SampledBit);

    if (flags.testFlag(SwapchainUsageFlagBits::InputAttachmentBit))
        result.setFlag(KDGpu::TextureUsageFlagBits::InputAttachmentBit);

    // Not clear how to map SwapchainUsageFlagBits::MutableFormatBit

    return result;
}

SessionState xrSessionStateToSessionState(XrSessionState state)
{
    return static_cast<SessionState>(static_cast<uint32_t>(state));
}

} // namespace KDXr

/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_framebuffer.h"

namespace KDGpu {

KDGpu::VulkanFramebuffer::VulkanFramebuffer(VkFramebuffer _framebuffer, const Handle<Device_t> &_deviceHandle)
    : framebuffer(_framebuffer)
    , deviceHandle(_deviceHandle)
{
}

} // namespace KDGpu

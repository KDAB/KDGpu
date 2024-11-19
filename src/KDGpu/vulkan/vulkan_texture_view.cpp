/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_texture_view.h"

namespace KDGpu {

VulkanTextureView::VulkanTextureView(VkImageView _imageView,
                                     const Handle<Texture_t> &_textureHandle,
                                     const Handle<Device_t> &_deviceHandle)
    : imageView(_imageView)
    , textureHandle(_textureHandle)
    , deviceHandle(_deviceHandle)
{
}

} // namespace KDGpu

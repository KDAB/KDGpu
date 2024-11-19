/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/texture_view.h>
#include <KDGpu/texture_view_options.h>
#include <KDGpu/kdgpu_export.h>

namespace KDGpu {

struct Device_t;
struct Texture_t;
struct TextureOptions;
class Device;
class Queue;

struct TextureSubresource {
    TextureAspectFlags aspectMask{ TextureAspectFlagBits::ColorBit };
    uint32_t mipLevel{ 0 };
    uint32_t arrayLayer{ 0 };
};

struct SubresourceLayout {
    DeviceSize offset{ 0 };
    DeviceSize size{ 0 };
    DeviceSize rowPitch{ 0 };
    DeviceSize arrayPitch{ 0 };
    DeviceSize depthPitch{ 0 };
};

/**
 * @brief Texture
 * @ingroup public
 */
class KDGPU_EXPORT Texture
{
public:
    Texture();
    ~Texture();

    Texture(Texture &&);
    Texture &operator=(Texture &&);

    Texture(const Texture &) = delete;
    Texture &operator=(const Texture &) = delete;

    const Handle<Texture_t> &handle() const noexcept { return m_texture; }
    bool isValid() const noexcept { return m_texture.isValid(); }

    operator Handle<Texture_t>() const noexcept { return m_texture; }

    TextureView createView(const TextureViewOptions &options = TextureViewOptions()) const;

    void *map();
    void unmap();

    SubresourceLayout getSubresourceLayout(const TextureSubresource &subresource = TextureSubresource()) const;

    bool generateMipMaps(Device &device, Queue &transferQueue, const TextureOptions &options, TextureLayout oldLayout, TextureLayout newLayout = TextureLayout::Undefined);

    MemoryHandle externalMemoryHandle() const;

private:
    explicit Texture(GraphicsApi *api, const Handle<Device_t> &device, const TextureOptions &options);
    explicit Texture(GraphicsApi *api, const Handle<Device_t> &device, const Handle<Texture_t> &handle); // From Swapchain

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<Texture_t> m_texture;

    void *m_mapped{ nullptr };

    friend class Swapchain;
    friend class Device;
    friend class VulkanGraphicsApi;
    friend KDGPU_EXPORT bool operator==(const Texture &, const Texture &);
};

KDGPU_EXPORT bool operator==(const Texture &a, const Texture &b);
KDGPU_EXPORT bool operator!=(const Texture &a, const Texture &b);

} // namespace KDGpu

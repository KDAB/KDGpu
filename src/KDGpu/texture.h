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

struct HostMemoryToTextureCopyRegion {
    void *srcHostMemoryPointer{ nullptr };
    DeviceSize srcMemoryRowLength{ 0 };
    DeviceSize srcMemoryImageHeight{ 0 };
    TextureSubresourceLayers dstSubresource{ .aspectMask = TextureAspectFlagBits::ColorBit };
    Offset3D dstOffset{};
    Extent3D dstExtent{};
};

struct TextureToHostMemoryCopyRegion {
    TextureSubresourceLayers srcSubresource{ .aspectMask = TextureAspectFlagBits::ColorBit };
    Offset3D srcOffset{};
    Extent3D srcExtent{};
    void *dstHostMemoryPointer{ nullptr };
    DeviceSize dstMemoryRowLength{ 0 };
    DeviceSize dstMemoryImageHeight{ 0 };
};

struct TextureToTextureHostCopyRegion {
    TextureSubresourceLayers srcSubresource{ .aspectMask = TextureAspectFlagBits::ColorBit };
    Offset3D srcOffset{};
    TextureSubresourceLayers dstSubresource{ .aspectMask = TextureAspectFlagBits::ColorBit };
    Offset3D dstOffset{};
    Extent3D extent{};
};

struct HostMemoryToTextureCopy {
    TextureLayout dstTextureLayout;
    std::vector<HostMemoryToTextureCopyRegion> regions;
    HostImageCopyFlags flags{ HostImageCopyFlagBits::None };
};

struct TextureToHostMemoryCopy {
    TextureLayout textureLayout;
    std::vector<TextureToHostMemoryCopyRegion> regions;
    HostImageCopyFlags flags{ HostImageCopyFlagBits::None };
};

struct TextureToTextureCopyHost {
    TextureLayout textureLayout;
    Handle<Texture_t> dstTexture;
    TextureLayout dstTextureLayout;
    std::vector<TextureToTextureHostCopyRegion> regions;
    HostImageCopyFlags flags{ HostImageCopyFlagBits::None };
};

struct HostLayoutTransition {
    TextureLayout oldLayout{ TextureLayout::Undefined };
    TextureLayout newLayout{ TextureLayout::Undefined };
    TextureSubresourceRange range{};
};

/*!
    \class Texture
    \brief Represents a GPU texture/image for rendering, sampling, and storage
    \ingroup public
    \headerfile texture.h <KDGpu/texture.h>

    <b>Vulkan equivalent:</b> [VkImage](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImage.html)

    Texture represents a multidimensional image on the GPU that can be used as a render target,
    sampled in shaders, or used for general storage. Textures can be 1D, 2D, 3D, or cube maps.

    <b>Key features:</b>
    - 1D, 2D, 3D, and cube map support
    - Mipmapping and multisampling
    - Various formats (color, depth, stencil, compressed)
    - Multiple usage modes (color attachment, depth, sampled, storage)
    - CPU-accessible mapping for linear textures
    .
    <br/>

    <b>Lifetime:</b> Textures are created by Device and must remain valid while referenced by pipelines,
    descriptor sets, or GPU commands. They use RAII and clean up automatically.

    ## Usage

    <b>Creating a 2D color texture:</b>

    \snippet kdgpu_doc_snippets.cpp texture_2d_creation

    <b>Creating a depth texture:</b>

    \snippet kdgpu_doc_snippets.cpp texture_depth_buffer

    <b>Creating a render target texture:</b>

    \snippet kdgpu_doc_snippets.cpp texture_render_target

    <b>Creating a cube map texture:</b>

    \snippet kdgpu_doc_snippets.cpp texture_cubemap

    <b>Creating a texture with mipmaps:</b>

    \snippet kdgpu_doc_snippets.cpp texture_mipmaps

    <b>Creating a multisampled texture (MSAA):</b>

    \snippet kdgpu_doc_snippets.cpp texture_multisampled

    <b>Resolving MSAA texture to non-MSAA texture:</b>

    \snippet kdgpu_doc_snippets.cpp texture_resolve

    <b>Creating a storage image for compute shaders:</b>

    \snippet kdgpu_doc_snippets.cpp texture_storage_image

    <b>Creating texture views for sampling:</b>

    \snippet kdgpu_doc_snippets.cpp texture_views

    <b>Uploading texture data:</b>

    \snippet kdgpu_doc_snippets.cpp texture_upload

    <b>Common texture formats:</b>

    \snippet kdgpu_doc_snippets.cpp texture_formats

    ## Vulkan mapping:
    - Texture creation -> vkCreateImage() + vkAllocateMemory() + vkBindImageMemory()
    - Texture::createView() -> vkCreateImageView()
    - Texture::generateMipMaps() -> vkCmdBlitImage() for each mip level
    - Texture::map() -> vkMapMemory() (for linear tiling)

    ## See also:
    \sa TextureView, Device, TextureOptions, Sampler
    \sa \ref kdgpu_api_overview
    \sa \ref kdgpu_vulkan_mapping
 */
class KDGPU_EXPORT Texture
{
public:
    Texture();
    ~Texture();

    Texture(Texture &&) noexcept;
    Texture &operator=(Texture &&) noexcept;

    Texture(const Texture &) = delete;
    Texture &operator=(const Texture &) = delete;

    const Handle<Texture_t> &handle() const noexcept { return m_texture; }
    bool isValid() const noexcept { return m_texture.isValid(); }

    operator Handle<Texture_t>() const noexcept { return m_texture; }

    TextureView createView(const TextureViewOptions &options = TextureViewOptions()) const;

    void *map();
    void unmap();

    void hostLayoutTransition(const HostLayoutTransition &transition);
    void copyHostMemoryToTexture(const HostMemoryToTextureCopy &copy);
    void copyTextureToHostMemory(const TextureToHostMemoryCopy &copy);
    void copyTextureToTextureHost(const TextureToTextureCopyHost &copy);

    SubresourceLayout getSubresourceLayout(const TextureSubresource &subresource = TextureSubresource()) const;

    /**
     * @brief Generate mipmaps by copying from another texture and then generating mipmaps for this texture
     * @param device KDGpu Device
     * @param transferQueue KDGpu Transfer Queue
     * @param sourceTexture Texture to copy/blit from when creating the mipmaps
     * @param options Texture Options for the target texture
     * @param oldLayout Transitioning from this layout
     * @param newLayout Transitioning to this layout when the mip map creation is done
     * @return true when successful
     */
    bool generateMipMaps(Device &device, Queue &transferQueue, const Handle<Texture_t> &sourceTexture, const TextureOptions &options, TextureLayout oldLayout, TextureLayout newLayout = TextureLayout::Undefined);

    /**
     * @brief Generate mipmaps for this texture
     * @param device KDGpu Device
     * @param transferQueue KDGpu Transfer Queue
     * @param options Texture Options for the target texture
     * @param oldLayout Transitioning from this layout
     * @param newLayout Transitioning to this layout when the mip map creation is done
     * @return true when successful
     */
    bool generateMipMaps(Device &device, Queue &transferQueue, const TextureOptions &options, TextureLayout oldLayout, TextureLayout newLayout = TextureLayout::Undefined);

    MemoryHandle externalMemoryHandle() const;

    uint64_t drmFormatModifier() const;

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

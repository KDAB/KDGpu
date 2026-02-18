/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/graphics_api.h>

namespace KDGpu {

struct TextureView_t;

/*!
    \class TextureView
    \brief Provides a specific view into a texture for rendering or sampling
    \ingroup public
    \headerfile texture_view.h <KDGpu/texture_view.h>

    <b>Vulkan equivalent:</b> [VkImageView](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImageView.html)

    TextureView represents a way to access a texture's data. It can view the entire texture or just
    a subset (specific mip levels, array layers, or aspect). Views are required for using textures
    in rendering, sampling, or storage.

    <b>Key features:</b>
    - View specific mip levels or array layers
    - Reinterpret texture format
    - Access specific aspects (color, depth, stencil)
    - Required for binding textures to pipelines
    .
    <br/>

    <b>Lifetime:</b> TextureViews are created from Textures and must not outlive their parent texture.
    They use RAII and clean up automatically.

    ## Usage

    <b>Creating a default texture view:</b>

    \snippet kdgpu_doc_snippets.cpp textureview_default

    <b>Viewing a specific mip level:</b>

    \snippet kdgpu_doc_snippets.cpp textureview_mip_level

    <b>Cube map face views:</b>

    \snippet kdgpu_doc_snippets.cpp textureview_cubemap_faces

    <b>Depth and stencil views:</b>

    \snippet kdgpu_doc_snippets.cpp textureview_depth_stencil

    <b>Array texture views:</b>

    \snippet kdgpu_doc_snippets.cpp textureview_array_texture

    <b>Using views in render passes:</b>

    \snippet kdgpu_doc_snippets.cpp textureview_render_pass

    <b>Storage image views (compute shaders):</b>

    \snippet kdgpu_doc_snippets.cpp textureview_storage_image

    ## Vulkan mapping:
    - TextureView creation -> vkCreateImageView()
    - Used in VkDescriptorImageInfo
    - Used in VkFramebufferCreateInfo

    ## See also:
    \sa Texture, Sampler, BindGroup, Device
    \sa \ref kdgpu_api_overview
    \sa \ref kdgpu_vulkan_mapping
 */
class KDGPU_EXPORT TextureView
{
public:
    TextureView();
    ~TextureView();

    TextureView(TextureView &&) noexcept;
    TextureView &operator=(TextureView &&);

    TextureView(const TextureView &) = delete;
    TextureView &operator=(const TextureView &) = delete;

    const Handle<TextureView_t> handle() const noexcept { return m_textureView; }
    bool isValid() const noexcept { return m_textureView.isValid(); }

    operator Handle<TextureView_t>() const noexcept { return m_textureView; }

private:
    explicit TextureView(GraphicsApi *api, const Handle<TextureView_t> &textureView);

    GraphicsApi *m_api{ nullptr };
    Handle<TextureView_t> m_textureView;

    friend class Texture;
    friend KDGPU_EXPORT bool operator==(const TextureView &, const TextureView &);
};

KDGPU_EXPORT bool operator==(const TextureView &a, const TextureView &b);
KDGPU_EXPORT bool operator!=(const TextureView &a, const TextureView &b);

} // namespace KDGpu

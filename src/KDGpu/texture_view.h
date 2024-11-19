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

/**
 * @brief TextureView
 * @ingroup public
 */
class KDGPU_EXPORT TextureView
{
public:
    TextureView();
    ~TextureView();

    TextureView(TextureView &&);
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

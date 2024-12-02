/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "texture_view.h"

#include <KDGpu/api/graphics_api_impl.h>

namespace KDGpu {

TextureView::TextureView()
{
}

TextureView::TextureView(GraphicsApi *api, const Handle<TextureView_t> &textureView)
    : m_api(api)
    , m_textureView(textureView)
{
}

TextureView::~TextureView()
{
    if (isValid())
        m_api->resourceManager()->deleteTextureView(handle());
}

TextureView::TextureView(TextureView &&other) noexcept
{
    m_api = std::exchange(other.m_api, nullptr);
    m_textureView = std::exchange(other.m_textureView, {});
}

TextureView &TextureView::operator=(TextureView &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteTextureView(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_textureView = std::exchange(other.m_textureView, {});
    }
    return *this;
}

bool operator==(const TextureView &a, const TextureView &b)
{
    return a.m_api == b.m_api && a.m_textureView == b.m_textureView;
}

bool operator!=(const TextureView &a, const TextureView &b)
{
    return !(a == b);
}

} // namespace KDGpu

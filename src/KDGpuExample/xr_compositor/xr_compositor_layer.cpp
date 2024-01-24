/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "xr_compositor_layer.h"

#include <KDGpuExample/xr_example_engine_layer.h>

namespace KDGpuExample {

XrCompositorLayer::XrCompositorLayer(Type type)
    : m_type(type)
{
}

XrCompositorLayer::~XrCompositorLayer()
{
}

const Engine *XrCompositorLayer::engine() const noexcept
{
    return m_engineLayer->engine();
}

Engine *XrCompositorLayer::engine() noexcept
{
    return m_engineLayer->engine();
}

std::shared_ptr<spdlog::logger> XrCompositorLayer::logger() const noexcept
{
    return engineLayer()->logger();
}

void XrCompositorLayer::uploadBufferData(const KDGpu::BufferUploadOptions &options)
{
    engineLayer()->uploadBufferData(options);
}

void XrCompositorLayer::uploadTextureData(const KDGpu::TextureUploadOptions &options)
{
    engineLayer()->uploadTextureData(options);
}

} // namespace KDGpuExample

/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "xr_projection_layer.h"

namespace KDGpuExample {

XrProjectionLayer::XrProjectionLayer(const XrProjectionLayerOptions &options)
    : XrCompositorLayer(Type::Projection)
    , m_device(options.device)
    , m_queue(options.queue)
    , m_session(options.session)
{
}

XrProjectionLayer::~XrProjectionLayer()
{
}

void XrProjectionLayer::update()
{
}

} // namespace KDGpuExample

/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "xr_passthrough_layer.h"

#include <KDXr/session.h>
#include <KDXr/passthrough_layer_controller.h>

namespace KDGpuExample {

XrPassthroughLayer::XrPassthroughLayer(const XrPassthroughLayerOptions &options)
    : XrCompositorLayer(Type::Quad)
    , m_device(options.device)
    , m_queue(options.queue)
    , m_session(options.session)
{
}

XrPassthroughLayer::~XrPassthroughLayer()
{
}

void XrPassthroughLayer::initialize()
{
    m_layerController = m_session->createPassthroughLayer({});
}

void XrPassthroughLayer::cleanup()
{
}

bool XrPassthroughLayer::update(const KDXr::FrameState &)
{
    // Set up the passthrough layer
    // clang-format off
    m_passthroughCompositionLayer = {
        .type = KDXr::CompositionLayerType::PassThrough,
        .referenceSpace = m_referenceSpace,
        .flags = KDXr::CompositionLayerFlagBits::BlendTextureSourceAlphaBit | KDXr::CompositionLayerFlagBits::UnpremultiplyAlphaBit | KDXr::CompositionLayerFlagBits::CorrectChromaticAberrationBit,
        .passthroughLayer = m_layerController.handle()
    };
    // clang-format on

    return true;
}

void XrPassthroughLayer::setRunning(bool running)
{
    m_layerController.setRunning(running);
}

} // namespace KDGpuExample

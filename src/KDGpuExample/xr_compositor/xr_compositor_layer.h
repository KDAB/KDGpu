/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/handle.h>
#include <KDXr/compositor.h>
#include <KDGpuExample/kdgpuexample_export.h>

namespace KDXr {
struct ReferenceSpace_t;
}

namespace KDGpuExample {

class XrExampleEngineLayer;

class KDGPUEXAMPLE_EXPORT XrCompositorLayer
{
public:
    enum class Type : uint32_t {
        Projection,
        Quad,
        Cylinder,
        Cube,
        Equirect,
        PassThrough,
    };

    virtual ~XrCompositorLayer();

    Type type() const { return m_type; }

    void setReferenceSpace(const KDXr::Handle<KDXr::ReferenceSpace_t> &referenceSpace) noexcept { m_referenceSpace = referenceSpace; }
    KDXr::Handle<KDXr::ReferenceSpace_t> referenceSpace() const noexcept { return m_referenceSpace; }

protected:
    explicit XrCompositorLayer(Type type);
    virtual void update() = 0;
    virtual KDXr::CompositionLayer *compositionLayer() = 0;

    Type m_type;
    XrExampleEngineLayer *m_engineLayer{ nullptr };
    KDXr::Handle<KDXr::ReferenceSpace_t> m_referenceSpace;

    friend class XrExampleEngineLayer;
};

} // namespace KDGpuExample

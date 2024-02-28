/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/compositor.h>
#include <KDGpuExample/kdgpuexample_export.h>

#include <KDGpu/handle.h>
#include <KDGpu/queue.h>

#include <spdlog/spdlog.h>

namespace KDXr {
struct ReferenceSpace_t;
}

namespace KDGpuExample {

class XrExampleEngineLayer;
class Engine;

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

    // Not copyable
    XrCompositorLayer(const XrCompositorLayer &) = delete;
    XrCompositorLayer &operator=(const XrCompositorLayer &) = delete;

    // Moveable
    XrCompositorLayer(XrCompositorLayer &&) = default;
    XrCompositorLayer &operator=(XrCompositorLayer &&) = default;

    Type type() const { return m_type; }

    void setReferenceSpace(const KDGpu::Handle<KDXr::ReferenceSpace_t> &referenceSpace) noexcept { m_referenceSpace = referenceSpace; }
    KDGpu::Handle<KDXr::ReferenceSpace_t> referenceSpace() const noexcept { return m_referenceSpace; }

    const XrExampleEngineLayer *engineLayer() const noexcept { return m_engineLayer; }
    XrExampleEngineLayer *engineLayer() noexcept { return m_engineLayer; }
    const Engine *engine() const noexcept;
    Engine *engine() noexcept;

protected:
    explicit XrCompositorLayer(Type type);
    virtual void initialize() = 0;
    virtual void cleanup() = 0;
    virtual bool update(const KDXr::FrameState &frameState) = 0;
    virtual KDXr::CompositionLayer *compositionLayer() = 0;

    std::shared_ptr<spdlog::logger> logger() const noexcept;
    void uploadBufferData(const KDGpu::BufferUploadOptions &options);
    void uploadTextureData(const KDGpu::TextureUploadOptions &options);

    Type m_type;
    XrExampleEngineLayer *m_engineLayer{ nullptr };
    KDGpu::Handle<KDXr::ReferenceSpace_t> m_referenceSpace;

    friend class XrExampleEngineLayer;
};

} // namespace KDGpuExample

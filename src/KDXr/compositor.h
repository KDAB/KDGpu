/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/kdxr_core.h>

#include <KDGpu/gpu_core.h>
#include <KDGpu/handle.h>

#include <span>

namespace KDXr {

struct ReferenceSpace_t;
struct Swapchain_t;

// Layers for the compositor

enum class CompositionLayerType : uint32_t {
    Projection = 0, // ProjectionLayer
    Quad = 1,
    Cylinder = 2,
    Cube = 3,
    Equirect = 4,
    PassThrough = 5,
    MaxEnum = 0x7fffffff
};

struct CompositionLayer {
    CompositionLayerType type;
    KDGpu::Handle<ReferenceSpace_t> referenceSpace;
    CompositionLayerFlags flags;
};

struct SwapchainSubTexture {
    KDGpu::Handle<Swapchain_t> swapchain;
    KDGpu::Rect2D rect{};
    uint32_t arrayIndex{ 0 };
};

struct ProjectionLayerView {
    Pose pose{};
    FieldOfView fieldOfView{};
    SwapchainSubTexture swapchainSubTexture{};
};

struct ProjectionLayer {
    CompositionLayerType type;
    KDGpu::Handle<ReferenceSpace_t> referenceSpace;
    CompositionLayerFlags flags{ CompositionLayerFlagBits::MaxEnum };
    std::span<ProjectionLayerView> views;
};

struct QuadLayer {
    CompositionLayerType type;
    KDGpu::Handle<ReferenceSpace_t> referenceSpace;
    CompositionLayerFlags flags{ CompositionLayerFlagBits::MaxEnum };
    EyeVisibility eyeVisibility;
    SwapchainSubTexture swapchainSubTexture{};
    Pose pose{};
    KDGpu::Extent2Df size{};
};

struct CylinderLayer {
    CompositionLayerType type;
    KDGpu::Handle<ReferenceSpace_t> referenceSpace;
    CompositionLayerFlags flags{ CompositionLayerFlagBits::MaxEnum };
    EyeVisibility eyeVisibility;
    SwapchainSubTexture swapchainSubTexture{};
    Pose pose{};
    float radius{ 0.0f };
    float centralAngle{ 0.0f };
    float aspectRatio{ 1.0f };
};

struct CubeLayer {
    CompositionLayerType type;
    KDGpu::Handle<ReferenceSpace_t> referenceSpace;
    CompositionLayerFlags flags{ CompositionLayerFlagBits::MaxEnum };
    EyeVisibility eyeVisibility;
    KDGpu::Handle<Swapchain_t> swapchain;
    uint32_t arrayIndex{ 0 };
    Quaternion orientation{};
};

struct EndFrameOptions {
    Time displayTime{ 0 };
    EnvironmentBlendMode environmentBlendMode{ EnvironmentBlendMode::MaxEnum };
    std::span<CompositionLayer *> layers;
};

} // namespace KDXr

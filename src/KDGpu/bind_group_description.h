/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/gpu_core.h>
#include <KDGpu/pipeline_layout_options.h>

#include <cstring>
#include <span>

namespace KDGpu {

struct Buffer_t;
struct TextureView_t;
struct Sampler_t;

struct TextureViewBinding {
    Handle<TextureView_t> textureView{};
    Handle<Sampler_t> sampler{};
};

struct ImageBinding {
    // TODO: Complete
};

struct UniformBufferBinding {
    static constexpr uint32_t WholeSize = 0xffffffff;

    Handle<Buffer_t> buffer{};
    uint32_t offset{ 0 };
    uint32_t size{ WholeSize };
};

struct StorageBufferBinding {
    static constexpr uint32_t WholeSize = 0xffffffff;

    Handle<Buffer_t> buffer{};
    uint32_t offset{ 0 };
    uint32_t size{ WholeSize };
};

struct DynamicUniformBufferBinding {
    static constexpr uint32_t WholeSize = 0xffffffff;
    Handle<Buffer_t> buffer{};
    uint32_t offset{ 0 };
    uint32_t size{ WholeSize };
};

class BindingResource
{
public:
    BindingResource(const TextureViewBinding &textureView)
        : m_type(ResourceBindingType::CombinedImageSampler)
    {
        m_resource.textureView = textureView;
    }

    BindingResource(const ImageBinding &image)
        : m_type(ResourceBindingType::StorageImage)
    {
        m_resource.image = image;
    }

    BindingResource(const UniformBufferBinding &buffer)
        : m_type(ResourceBindingType::UniformBuffer)
    {
        m_resource.uniformBuffer = buffer;
    }

    BindingResource(const StorageBufferBinding &buffer)
        : m_type(ResourceBindingType::StorageBuffer)
    {
        m_resource.storageBuffer = buffer;
    }

    BindingResource(const DynamicUniformBufferBinding &buffer)
        : m_type(ResourceBindingType::DynamicUniformBuffer)
    {
        m_resource.dynamicUniformBuffer = buffer;
    }

    ResourceBindingType type() const { return m_type; }
    const UniformBufferBinding &uniformBufferBinding() const { return m_resource.uniformBuffer; }
    const StorageBufferBinding &storageBufferBinding() const { return m_resource.storageBuffer; }
    const ImageBinding &imageBinding() const { return m_resource.image; }
    const TextureViewBinding &textureViewBinding() const { return m_resource.textureView; }
    const DynamicUniformBufferBinding &dynamicUniformBufferBinding() const { return m_resource.dynamicUniformBuffer; }

private:
    union Resource {
        Resource() { std::memset(this, 0, sizeof(Resource)); }

        TextureViewBinding textureView;
        ImageBinding image;
        UniformBufferBinding uniformBuffer;
        StorageBufferBinding storageBuffer;
        DynamicUniformBufferBinding dynamicUniformBuffer;
    } m_resource;
    ResourceBindingType m_type;
};

} // namespace KDGpu

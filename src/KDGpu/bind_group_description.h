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

namespace KDGpu {

struct Buffer_t;
struct TextureView_t;
struct Sampler_t;
struct AccelerationStructure_t;

struct TextureViewSamplerBinding {
    Handle<TextureView_t> textureView{};
    Handle<Sampler_t> sampler{};
    TextureLayout layout{ TextureLayout::ShaderReadOnlyOptimal };
};

struct TextureViewBinding {
    Handle<TextureView_t> textureView{};
    TextureLayout layout{ TextureLayout::ShaderReadOnlyOptimal };
};

struct InputAttachmentBinding {
    Handle<TextureView_t> textureView{};
    TextureLayout layout{ TextureLayout::ShaderReadOnlyOptimal };
};

struct SamplerBinding {
    Handle<Sampler_t> sampler{};
};

struct ImageBinding {
    Handle<TextureView_t> textureView{};
    TextureLayout layout{ TextureLayout::General };
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

struct AccelerationStructureBinding {
    Handle<AccelerationStructure_t> accelerationStructure{};
};

class BindingResource
{
public:
    BindingResource(const TextureViewSamplerBinding &textureView)
        : m_type(ResourceBindingType::CombinedImageSampler)
    {
        m_resource.combineTextureViewSampler = textureView;
    }

    BindingResource(const TextureViewBinding &textureView)
        : m_type(ResourceBindingType::SampledImage)
    {
        m_resource.textureView = textureView;
    }

    BindingResource(const ImageBinding &image)
        : m_type(ResourceBindingType::StorageImage)
    {
        m_resource.image = image;
    }

    BindingResource(const SamplerBinding &sampler)
        : m_type(ResourceBindingType::Sampler)
    {
        m_resource.sampler = sampler;
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

    BindingResource(const AccelerationStructureBinding &buffer)
        : m_type(ResourceBindingType::AccelerationStructure)
    {
        m_resource.accelerationStructure = buffer;
    }

    BindingResource(const InputAttachmentBinding &inputAttachment)
        : m_type(ResourceBindingType::InputAttachment)
    {
        m_resource.inputAttachment = inputAttachment;
    }

    ResourceBindingType type() const { return m_type; }
    const UniformBufferBinding &uniformBufferBinding() const { return m_resource.uniformBuffer; }
    const StorageBufferBinding &storageBufferBinding() const { return m_resource.storageBuffer; }
    const ImageBinding &imageBinding() const { return m_resource.image; }
    const SamplerBinding &samplerBinding() const { return m_resource.sampler; }
    const TextureViewBinding &textureViewBinding() const { return m_resource.textureView; }
    const TextureViewSamplerBinding &textureViewSamplerBinding() const { return m_resource.combineTextureViewSampler; }
    const DynamicUniformBufferBinding &dynamicUniformBufferBinding() const { return m_resource.dynamicUniformBuffer; }
    const AccelerationStructureBinding &accelerationStructure() const { return m_resource.accelerationStructure; }
    const InputAttachmentBinding &inputAttachmentBinding() const { return m_resource.inputAttachment; }

private:
    union Resource {
        Resource() { std::memset(this, 0, sizeof(Resource)); }

        TextureViewSamplerBinding combineTextureViewSampler;
        TextureViewBinding textureView;
        ImageBinding image;
        SamplerBinding sampler;
        UniformBufferBinding uniformBuffer;
        StorageBufferBinding storageBuffer;
        DynamicUniformBufferBinding dynamicUniformBuffer;
        AccelerationStructureBinding accelerationStructure;
        InputAttachmentBinding inputAttachment;
    } m_resource;
    ResourceBindingType m_type;
};

} // namespace KDGpu

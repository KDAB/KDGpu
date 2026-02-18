/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/bind_group_description.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/graphics_api.h>

namespace KDGpu {

struct BindGroupEntry;
struct BindGroup_t;
struct Device_t;
struct BindGroupOptions;

// A BindGroup is what is known as a descriptor set in Vulkan parlance. Other APIs such
// as web-gpu call them bind groups which to me helps with the mental model a little more.
//

/*!
    \class BindGroup
    \brief Represents a set of shader resource bindings (descriptor set)
    \ingroup public
    \headerfile bind_group.h <KDGpu/bind_group.h>

    <b>Vulkan equivalent:</b> [VkDescriptorSet](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorSet.html)

    BindGroup (also known as descriptor set in Vulkan) groups together shader resources like buffers,
    textures, and samplers. Shaders declare what resources they need, and bind groups provide the
    actual resources.

    <b>Key features:</b>
    - Group multiple resources for efficient binding
    - Update individual bindings dynamically
    - Allocate from descriptor pools
    - Match shader layout declarations
    .
    <br/>

    <b>Lifetime:</b>
    BindGroups are allocated from a BindGroupPool and must remain valid while
    referenced by command buffers. They use RAII and clean up automatically.

    Note that KDGpu uses a default internal BindGroupPool if you do not specify one, so you can create bind groups without managing pools directly.

    <b>Available BindingResource bindings:</b>
    - TextureViewSamplerBinding
    - TextureViewBinding
    - InputAttachmentBinding
    - SamplerBinding
    - ImageBinding
    - UniformBufferBinding
    - StorageBufferBinding
    - DynamicUniformBufferBinding
    - AccelerationStructureBinding

    ## Usage

    <b>Creating a simple bind group:</b>

    \snippet kdgpu_doc_snippets.cpp bindgroup_simple

    <b>Using bind groups in rendering:</b>

    \snippet kdgpu_doc_snippets.cpp bindgroup_usage

    <b>Updating bind groups:</b>

    \snippet kdgpu_doc_snippets.cpp bindgroup_update

    <b>In some cases, you might need to create the BindGroup without binding it to a resource immediately:</b>

    \snippet kdgpu_doc_snippets.cpp bindgroup_resource_bound_after_creation

    <b>Multiple uniform buffers:</b>

    \snippet kdgpu_doc_snippets.cpp bindgroup_multiple_uniforms

    <b>Multiple bind groups (different sets):</b>

    \snippet kdgpu_doc_snippets.cpp bindgroup_pipeline_layout

    <b>Dynamic offset binding:</b>

    \snippet kdgpu_doc_snippets.cpp bindgroup_dynamic_offset

    <b>Storage buffer binding:</b>

    \snippet kdgpu_doc_snippets.cpp bindgroup_storage

    <b>Separate TextureView and Sampler binding:</b>

    \snippet kdgpu_doc_snippets.cpp bindgroup_texture_view_sampler_separate

    ## Vulkan mapping:
    - BindGroup creation->vkAllocateDescriptorSets()
    - BindGroup::update()->vkUpdateDescriptorSets()
    - RenderPassCommandRecorder::setBindGroup()->vkCmdBindDescriptorSets()

    ## See also:
    \sa BindGroupLayout, BindGroupOptions, BindGroupPool, Device, PipelineLayout, BindingResource, BindGroupEntry
    \sa \ref kdgpu_api_overview
    \sa \ref kdgpu_vulkan_mapping
*/
class KDGPU_EXPORT BindGroup
{
public:
    BindGroup();
    ~BindGroup();

    BindGroup(BindGroup &&) noexcept;
    BindGroup &operator=(BindGroup &&) noexcept;

    BindGroup(const BindGroup &) = delete;
    BindGroup &operator=(const BindGroup &) = delete;

    const Handle<BindGroup_t> &handle() const noexcept { return m_bindGroup; }
    bool isValid() const;

    operator Handle<BindGroup_t>() const noexcept { return m_bindGroup; }

    void update(const BindGroupEntry &entry);

private:
    explicit BindGroup(GraphicsApi *api, const Handle<Device_t> &device, const BindGroupOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<BindGroup_t> m_bindGroup;

    friend class Device;
    friend KDGPU_EXPORT bool operator==(const BindGroup &, const BindGroup &);
};

KDGPU_EXPORT bool operator==(const BindGroup &a, const BindGroup &b);
KDGPU_EXPORT bool operator!=(const BindGroup &a, const BindGroup &b);

} // namespace KDGpu

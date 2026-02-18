/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/graphics_api.h>

namespace KDGpu {

struct BindGroupLayout_t;
struct Device_t;
struct BindGroupLayoutOptions;

/*!
    \class BindGroupLayout
    \brief Describes the structure of shader resource bindings (descriptor set layout)
    \ingroup public
    \headerfile bind_group_layout.h <KDGpu/bind_group_layout.h>

    <b>Vulkan equivalent:</b> [VkDescriptorSetLayout](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorSetLayout.html)

    BindGroupLayout defines the types and configuration of resources that shaders expect at each
    binding point. It's the template from which actual BindGroups are created.

    <b>Key features:</b>
    - Declare binding types (uniform, storage, texture, sampler)
    - Specify shader stages that access each binding
    - Define binding numbers matching shader declarations
    - Reusable template for creating bind groups
    .
    <br/>

    <b>Lifetime:</b> BindGroupLayouts are created by Device and must remain valid while referenced
    by pipeline layouts or bind groups. They use RAII and clean up automatically.

    ## Usage

    <b>Creating a basic bind group layout:</b>

    \snippet kdgpu_doc_snippets.cpp bindgrouplayout_simple

    <b>Multi-stage access(vertex + fragment):</b>

    \snippet kdgpu_doc_snippets.cpp bindgrouplayout_shader_stages

    <b>Storage buffer layout(compute shader):</b>

    \snippet kdgpu_doc_snippets.cpp bindgrouplayout_compute

    <b>Texture array binding:</b>

    \snippet kdgpu_doc_snippets.cpp bindgrouplayout_multiple_descriptors

    <b>Storage image layout(compute output):</b>

    \snippet kdgpu_doc_snippets.cpp bindgrouplayout_storage_image

    <b>Dynamic uniform buffer:</b>

    \snippet kdgpu_doc_snippets.cpp bindgrouplayout_dynamic_ubo

    <b>Bindless rendering (large descriptor arrays):</b>

    \snippet kdgpu_doc_snippets.cpp bindgrouplayout_bindless

    <b>Checking layout compatibility:</b>

    \snippet kdgpu_doc_snippets.cpp bindgrouplayout_compatibility

    <b>Corresponding shader code(GLSL):</b> Shader declarations must match bind group layout bindings. Example showing multiple descriptor sets:

    \code{.glsl}
    // Set 0 (frame data)
    layout(set = 0, binding = 0) uniform FrameUBO
    {
        mat4 view;
        mat4 projection;
    } frame;

    // Set 1 (material data)
    layout(set = 1, binding = 0) uniform sampler2D albedoMap;
    layout(set = 1, binding = 1) uniform sampler2D normalMap;
    layout(set = 1, binding = 2) uniform sampler2D roughnessMap;

    // Set 2 (object data)
    layout(set = 2, binding = 0) uniform ObjectUBO
    {
        mat4 model;
    } object;
    \endcode

    would match with:

    \snippet kdgpu_doc_snippets.cpp bindgrouplayout_shader_match


    ## Vulkan mapping:
    - BindGroupLayout creation->vkCreateDescriptorSetLayout()
    - Used in VkPipelineLayoutCreateInfo
    - Used in VkDescriptorSetAllocateInfo

    ## See also:
    \sa BindGroup, BindGroupLayoutOptions, PipelineLayout, Device, BindGroupPool
    \sa \ref kdgpu_api_overview
    \sa \ref kdgpu_vulkan_mapping
*/
class KDGPU_EXPORT BindGroupLayout
{
public:
    BindGroupLayout();
    ~BindGroupLayout();

    BindGroupLayout(BindGroupLayout &&) noexcept;
    BindGroupLayout &operator=(BindGroupLayout &&) noexcept;

    BindGroupLayout(const BindGroupLayout &) = delete;
    BindGroupLayout &operator=(const BindGroupLayout &) = delete;

    const Handle<BindGroupLayout_t> &handle() const noexcept { return m_bindGroupLayout; }
    bool isValid() const noexcept { return m_bindGroupLayout.isValid(); }

    operator Handle<BindGroupLayout_t>() const noexcept { return m_bindGroupLayout; }

    /*!
        Checks whether this BindGroupLayout is compatible with \a other.
    */
    bool isCompatibleWith(const Handle<BindGroupLayout_t> &other) const;

private:
    explicit BindGroupLayout(GraphicsApi *api, const Handle<Device_t> &device, const BindGroupLayoutOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<BindGroupLayout_t> m_bindGroupLayout;

    friend class Device;
    friend KDGPU_EXPORT bool operator==(const BindGroupLayout &, const BindGroupLayout &);
};

KDGPU_EXPORT bool operator==(const BindGroupLayout &a, const BindGroupLayout &b);
KDGPU_EXPORT bool operator!=(const BindGroupLayout &a, const BindGroupLayout &b);

} // namespace KDGpu

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

struct Device_t;
struct ShaderModule_t;

/*!
    \class ShaderModule
    \brief Represents compiled shader code (SPIR-V) for a pipeline stage
    \ingroup public
    \headerfile shader_module.h <KDGpu/shader_module.h>

    <b>Vulkan equivalent:</b> [VkShaderModule](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkShaderModule.html)

    ShaderModule wraps compiled SPIR-V shader bytecode. Shaders are written in GLSL or HLSL, compiled
    to SPIR-V format, and then loaded into shader modules for use in graphics or compute pipelines.

    <b>Key features:</b>
    - Encapsulates SPIR-V bytecode
    - Reusable across multiple pipelines
    - Supports all shader stages
    - Entry point specification
    .
    <br/>

    <b>Lifetime:</b> ShaderModules are created by Device and must remain valid during pipeline creation. ShaderModules can be destroyed after all pipelines that use them have been created.
    They use RAII and clean up automatically.

    ## Usage

    <b>Loading shader from file:</b>

    \snippet kdgpu_doc_snippets.cpp load_spirv

    \snippet kdgpu_doc_snippets.cpp shadermodule_from_spirv_file

    \sa KDGpuExample::readShaderFile

    <b>Loading shader from SPIR-V data in memory:</b>

    \snippet kdgpu_doc_snippets.cpp shadermodule_from_spirv_memory

    <b>Creating graphics pipeline with shaders:</b>

    \snippet kdgpu_doc_snippets.cpp shadermodule_pipeline_usage

    <b>Creating compute pipeline with shader:</b>

    \snippet kdgpu_doc_snippets.cpp shadermodule_compute

    <b>Using specialization constants:</b>

    \snippet kdgpu_doc_snippets.cpp shadermodule_specialization

    <b>Reusing shaders across pipelines:</b>

    \snippet kdgpu_doc_snippets.cpp shadermodule_reuse

    <b>Geometry shader example:</b>

    \snippet kdgpu_doc_snippets.cpp shadermodule_geometry

    <b>Compiling GLSL to SPIR-V:</b>

    \code{.sh}
    # Using glslangValidator
    glslangValidator -V shader.vert -o shader.vert.spv
    glslangValidator -V shader.frag -o shader.frag.spv
    glslangValidator -V shader.comp -o shader.comp.spv

    # Using glslc (from shaderc)
    glslc shader.vert -o shader.vert.spv
    glslc shader.frag -o shader.frag.spv

    # Using KDGpu cmake helpers in a CMakeLists.txt
    kdgpu_compileshader(ComputeShaderTarget particles.comp particles.comp.spv)
    kdgpu_compileshaderset(VertexFragmentShaderTarget vertex) # looks for vertex.vert and vertex.frag, compiles to vertex.vert.spv and vertex.frag.spv
    kdgpu_compilegeomshaderset(VertexGeomFragmentShaderTarget geom) # looks for geom.vert geom.geom and geom.frag, compiles to geom.vert.spv, geom.geom.spv and geom.frag.spv
    kdgpu_compilemeshtaskshader(MeshShaderTarget mesh_shader.mesh mesh_shader.mesh.spv)
    kdgpu_compilertshader(RtGenShaderTarget raygen.rgen raygen.spv)
    \endcode

    ## Vulkan mapping:
    - ShaderModule creation -> vkCreateShaderModule()
    - Used in VkPipelineShaderStageCreateInfo

    ## See also:
    \sa GraphicsPipeline, ComputePipeline, Device, PipelineLayout
    \sa \ref kdgpu_api_overview
    \sa \ref kdgpu_vulkan_mapping
 */
class KDGPU_EXPORT ShaderModule
{
public:
    ShaderModule();
    ~ShaderModule();

    ShaderModule(ShaderModule &&) noexcept;
    ShaderModule &operator=(ShaderModule &&) noexcept;

    ShaderModule(const ShaderModule &) = delete;
    ShaderModule &operator=(const ShaderModule &) = delete;

    Handle<ShaderModule_t> handle() const noexcept { return m_shaderModule; }
    bool isValid() const noexcept { return m_shaderModule.isValid(); }

    operator Handle<ShaderModule_t>() const noexcept { return m_shaderModule; }

private:
    ShaderModule(GraphicsApi *api, const Handle<Device_t> &device, const std::vector<uint32_t> &code);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<ShaderModule_t> m_shaderModule;

    friend class Device;
};

} // namespace KDGpu

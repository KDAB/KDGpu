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
struct Sampler_t;
struct SamplerOptions;

/*!
    \class Sampler
    \brief Defines how textures are sampled in shaders
    \ingroup public
    \headerfile sampler.h <KDGpu/sampler.h>

    <b>Vulkan equivalent:</b> [VkSampler](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSampler.html)

    Sampler controls how texture data is read in shaders: filtering modes, addressing modes for
    coordinates outside [0,1], anisotropic filtering, and more.

    <b>Key features:</b>
    - Minification and magnification filtering
    - Mipmap filtering
    - Address modes (repeat, clamp, mirror)
    - Anisotropic filtering
    - Comparison operations for shadow mapping
    .
    <br/>

    <b>Lifetime:</b> Samplers are created by Device and must remain valid while referenced by
    bind groups. They use RAII and clean up automatically.

    ## Usage

    <b>Creating a basic sampler:</b>

    \snippet kdgpu_doc_snippets.cpp sampler_basic

    <b>Nearest-neighbor filtering (pixel art):</b>

    \snippet kdgpu_doc_snippets.cpp sampler_nearest

    <b>Linear filtering:</b>

    \snippet kdgpu_doc_snippets.cpp sampler_linear

    <b>Anisotropic filtering (high quality):</b>

    \snippet kdgpu_doc_snippets.cpp sampler_anisotropic

    <b>Clamp to edge sampler:</b>

    \snippet kdgpu_doc_snippets.cpp sampler_edge

    <b>Clamp to border with custom color:</b>

    \snippet kdgpu_doc_snippets.cpp sampler_border

    <b>Common addressing modes:</b>

    \code{.cpp}
    // Repeat: Texture tiles infinitely
    KDGpu::AddressMode::Repeat

    // Mirror: Texture mirrors at each integer boundary
    KDGpu::AddressMode::MirroredRepeat

    // Clamp to edge: Extends edge pixels
    KDGpu::AddressMode::ClampToEdge

    // Clamp to border: Uses border color outside [0,1]
    KDGpu::AddressMode::ClampToBorder
    \endcode

    <b>Mipmap LOD control:</b>

    \snippet kdgpu_doc_snippets.cpp sampler_lod

    <b>Reusing samplers:</b>

    \snippet kdgpu_doc_snippets.cpp sampler_reuse

    <b>Corresponding shader code (GLSL):</b>

    \code{.cpp}
    // Combined image sampler
    layout(set = 1, binding = 0) uniform sampler2D diffuseTexture;

    void main() {
        vec4 color = texture(diffuseTexture, texCoord);
        // Sampler settings (filtering, addressing) applied automatically
    }

    // Separate texture and sampler
    layout(set = 1, binding = 0) uniform sampler edgeSampler;
    layout(set = 1, binding = 1) uniform texture2D colorTex;

    void main() {
        vec4 color = texture(samplee2D(colorTex, edgeSampler), texCoord);
    }
    \endcode

    ## Vulkan mapping:
    - Sampler creation -> vkCreateSampler()
    - Used in VkDescriptorImageInfo with texture views

    ## See also
    \sa SamplerOptions, TextureView, BindGroup, Device, Texture
    \sa \ref kdgpu_api_overview
    \sa \ref kdgpu_vulkan_mapping
 */
class KDGPU_EXPORT Sampler
{
public:
    Sampler();
    ~Sampler();

    Sampler(Sampler &&) noexcept;
    Sampler &operator=(Sampler &&) noexcept;

    Sampler(const Sampler &) = delete;
    Sampler &operator=(const Sampler &) = delete;

    Handle<Sampler_t> handle() const noexcept { return m_sampler; }
    bool isValid() const noexcept { return m_sampler.isValid(); }

    operator Handle<Sampler_t>() const noexcept { return m_sampler; }

private:
    Sampler(GraphicsApi *api, const Handle<Device_t> &device, const SamplerOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<Sampler_t> m_sampler;

    friend class Device;
    friend KDGPU_EXPORT bool operator==(const Sampler &, const Sampler &);
};

KDGPU_EXPORT bool operator==(const Sampler &a, const Sampler &b);
KDGPU_EXPORT bool operator!=(const Sampler &a, const Sampler &b);

} // namespace KDGpu

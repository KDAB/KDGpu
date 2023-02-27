#pragma once

#include <toy_renderer/handle.h>
#include <toy_renderer/gpu_core.h>
#include <toy_renderer/pipeline_layout_options.h>

#include <cstring>
#include <span>

namespace ToyRenderer {

struct Buffer_t;
struct TextureView_t;
struct Sampler_t;

struct TextureViewBinding {
    Handle<TextureView_t> textureView{};
    // Handle<Sampler_t> sampler{};
    // TODO: Use TextureView instead of Texture?
    // TODO: Add support for samplers.
};

struct ImageBinding {
    // TODO: Complete
};

struct BufferBinding {
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

    BindingResource(const BufferBinding &buffer)
        : m_type(ResourceBindingType::UniformBuffer) // TODO: How do we distinguish from SSBO...
    {
        m_resource.buffer = buffer;
    }

    ResourceBindingType type() const { return m_type; }

private:
    union Resource {
        Resource() { std::memset(this, 0, sizeof(Resource)); }

        TextureViewBinding textureView;
        ImageBinding image;
        BufferBinding buffer;
    } m_resource;
    ResourceBindingType m_type;
};

} // namespace ToyRenderer

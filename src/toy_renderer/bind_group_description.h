#pragma once

#include <toy_renderer/handle.h>

#include <span>

namespace ToyRenderer {

class Buffer;
class Texture;

struct TextureBinding {
    Handle<Texture> texture{};
    // TODO: Use TextureView instead of Texture?
    // TODO: Add support for samplers.
    uint32_t slot{ 0 };
};

struct BufferBinding {
    static constexpr uint32_t WholeSize = 0xffffffff;

    Handle<Buffer> buffer{};
    uint32_t offset{ 0 };
    uint32_t size{ WholeSize };
    uint32_t slot{ 0 };
};

// A BindGroup is what is known as a descriptor set in Vulkan parlance. Other APIs such
// as web-gpu call them bind groups which to me helps with the mental model a little more.
//
// The following struct describes a bind group (descriptor set) layout and from this we
// will be able to subsequently allocate the actual bind group (descriptor set). Before
// the bind group can be used we will need to populate it with the specified bindings.
struct BindGroupDescription {
    std::span<TextureBinding> textures;
    std::span<BufferBinding> buffers;
};

} // namespace ToyRenderer

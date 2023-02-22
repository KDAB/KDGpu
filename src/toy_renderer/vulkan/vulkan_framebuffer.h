#pragma once

#include <toy_renderer/api/api_framebuffer.h>

#include <toy_renderer/handle.h>
#include <toy_renderer/utils/hash_utils.h>

#include <vulkan/vulkan.h>

#include <vector>

namespace ToyRenderer {

struct Framebuffer_t;
struct RenderPass_t;
struct TextureView_t;

struct VulkanFramebufferKey {
    Handle<RenderPass_t> renderPass;
    std::vector<Handle<TextureView_t>> attachments;
    uint32_t width{ 0 };
    uint32_t height{ 0 };
    uint32_t layers{ 0 };

    bool operator==(const VulkanFramebufferKey &other) const noexcept
    {
        // clang-format off
        return renderPass == other.renderPass
            && attachments == other.attachments
            && width == other.width
            && height == other.height
            && layers == other.layers;
        // clang-format on
    }

    bool operator!=(const VulkanFramebufferKey &other) const noexcept
    {
        return !(*this == other);
    }
};

struct VulkanFramebuffer : public ApiFramebuffer {
    explicit VulkanFramebuffer(VkFramebuffer _framebuffer);

    VkFramebuffer framebuffer{ VK_NULL_HANDLE };
};

} // namespace ToyRenderer

namespace std {

template<>
struct hash<ToyRenderer::VulkanFramebufferKey> {
    size_t operator()(const ToyRenderer::VulkanFramebufferKey &value) const
    {
        uint64_t hash = 0;

        ToyRenderer::hash_combine(hash, value.renderPass);

        for (const auto &attachment : value.attachments)
            ToyRenderer::hash_combine(hash, attachment);

        ToyRenderer::hash_combine(hash, value.width);
        ToyRenderer::hash_combine(hash, value.height);
        ToyRenderer::hash_combine(hash, value.layers);

        return hash;
    }
};

} // namespace std

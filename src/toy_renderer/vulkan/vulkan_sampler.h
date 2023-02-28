#pragma once

#include <toy_renderer/api/api_sampler.h>
#include <vulkan/vulkan.h>

namespace ToyRenderer {

class VulkanResourceManager;
struct Device_t;

struct VulkanSampler : public ApiSampler {

    explicit VulkanSampler(VkSampler _sampler);

    VkSampler sampler{ VK_NULL_HANDLE };
};

} // namespace ToyRenderer

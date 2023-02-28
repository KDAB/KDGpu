#pragma once

#include <toy_renderer/api/api_sampler.h>
#include <toy_renderer/handle.h>
#include <vulkan/vulkan.h>

namespace ToyRenderer {

class VulkanResourceManager;
struct Device_t;

struct VulkanSampler : public ApiSampler {

    explicit VulkanSampler(VkSampler _sampler, const Handle<Device_t> &_deviceHandle);

    VkSampler sampler{ VK_NULL_HANDLE };
    Handle<Device_t> deviceHandle;
};

} // namespace ToyRenderer

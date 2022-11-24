#pragma once

#include <toy_renderer/api/api_shader_module.h>

#include <toy_renderer/handle.h>

#include <vulkan/vulkan.h>

namespace ToyRenderer {

class VulkanResourceManager;

struct Device_t;

struct VulkanShaderModule : public ApiShaderModule {
    explicit VulkanShaderModule(VkShaderModule _shaderModule,
                                VulkanResourceManager *_vulkanResourceManager,
                                const Handle<Device_t> _deviceHandle);

    VkShaderModule shaderModule{ VK_NULL_HANDLE };
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
};

} // namespace ToyRenderer

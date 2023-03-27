#pragma once

#include <kdgpu/api/api_shader_module.h>

#include <kdgpu/handle.h>

#include <vulkan/vulkan.h>

namespace KDGpu {

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

} // namespace KDGpu

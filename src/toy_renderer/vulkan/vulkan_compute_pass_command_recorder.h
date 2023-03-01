#pragma once

#include <toy_renderer/api/api_compute_pass_command_recorder.h>
#include <toy_renderer/handle.h>
#include <vulkan/vulkan.h>

namespace ToyRenderer {

class VulkanResourceManager;

struct ComputePipeline_t;
struct Device_t;

struct VulkanComputePassCommandRecorder : public ApiComputePassCommandRecorder {

    explicit VulkanComputePassCommandRecorder(VkCommandBuffer _commandBuffer,
                                              VulkanResourceManager *_vulkanResourceManager,
                                              const Handle<Device_t> &_deviceHandle);

    void setPipeline(const Handle<ComputePipeline_t> &pipeline) final;
    void setBindGroup(uint32_t group, const Handle<BindGroup_t> &bindGroup) final;
    void dispatchCompute(const ComputeCommand &command) final;
    void dispatchCompute(const std::vector<ComputeCommand> &commands) final;
    void dispatchComputeIndirect(const ComputeCommandIndirect &command) final;
    void dispatchComputeIndirect(const std::vector<ComputeCommandIndirect> &commands) final;
    void pushConstant(const PushConstantRange &constantRange, const std::vector<uint8_t> &data) final;
    void end() final;

    VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
    Handle<ComputePipeline_t> pipeline;
};

} // namespace ToyRenderer

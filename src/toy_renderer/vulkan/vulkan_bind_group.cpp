#include "vulkan_bind_group.h"
#include <toy_renderer/bind_group_options.h>
#include <toy_renderer/vulkan/vulkan_device.h>
#include <toy_renderer/vulkan/vulkan_resource_manager.h>

namespace ToyRenderer {

VulkanBindGroup::VulkanBindGroup(VkDescriptorSet _descriptorSet,
                                 VulkanResourceManager *_vulkanResourceManager,
                                 const Handle<Device_t> &_deviceHandle)
    : descriptorSet(_descriptorSet)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
}

void VulkanBindGroup::update(const BindGroupEntry &entry)
{
    VulkanDevice *vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);

    VkDescriptorBufferInfo uboBufferInfo{};

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = entry.binding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorCount = 0;
    descriptorWrite.pImageInfo = nullptr;
    descriptorWrite.pTexelBufferView = nullptr;
    descriptorWrite.pBufferInfo = nullptr;
    descriptorWrite.pTexelBufferView = nullptr;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    switch (entry.resource.type()) {
    case ResourceBindingType::CombinedImageSampler: {
        const TextureViewBinding &textureViewBinding = entry.resource.textureViewBinding();
        VulkanTextureView *textView = vulkanResourceManager->getTextureView(textureViewBinding.textureView);
        // VulkanSampler *sampler = m_samplers.get(textureViewBinding.sampler);
        imageInfo.imageView = textView->imageView;
        // TODO: Create Sampler
        // imageInfo.sampler = vulkanTextureView.sampler;

        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;
        break;
    }
    case ResourceBindingType::UniformBuffer:
        const BufferBinding &bufferBinding = entry.resource.bufferBinding();
        VulkanBuffer *buffer = vulkanResourceManager->getBuffer(bufferBinding.buffer);
        uboBufferInfo.buffer = buffer->buffer; // VkBuffer
        uboBufferInfo.offset = bufferBinding.offset;
        uboBufferInfo.range = (bufferBinding.size == BufferBinding::WholeSize) ? VK_WHOLE_SIZE : bufferBinding.size;

        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &uboBufferInfo;
        break;
    }

    if (descriptorWrite.descriptorCount > 0)
        vkUpdateDescriptorSets(vulkanDevice->device, 1, &descriptorWrite, 0, nullptr);
}

} // namespace ToyRenderer

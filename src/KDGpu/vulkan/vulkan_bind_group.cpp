/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_bind_group.h"
#include <KDGpu/bind_group_options.h>
#include <KDGpu/vulkan/vulkan_device.h>
#include <KDGpu/vulkan/vulkan_resource_manager.h>

namespace KDGpu {

VulkanBindGroup::VulkanBindGroup(VkDescriptorSet _descriptorSet,
                                 VkDescriptorPool _descriptorPool,
                                 VulkanResourceManager *_vulkanResourceManager,
                                 const Handle<Device_t> &_deviceHandle)
    : descriptorSet(_descriptorSet)
    , descriptorPool(_descriptorPool)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
}

void VulkanBindGroup::update(const BindGroupEntry &entry)
{
    VulkanDevice *vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);

    VkDescriptorBufferInfo bufferInfo{};

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = entry.binding;
    descriptorWrite.dstArrayElement = entry.arrayElement;
    descriptorWrite.descriptorCount = 0;
    descriptorWrite.pImageInfo = nullptr;
    descriptorWrite.pTexelBufferView = nullptr;
    descriptorWrite.pBufferInfo = nullptr;
    descriptorWrite.pTexelBufferView = nullptr;

    switch (entry.resource.type()) {
    case ResourceBindingType::CombinedImageSampler: {
        const TextureViewSamplerBinding &textureViewBinding = entry.resource.textureViewSamplerBinding();
        VulkanTextureView *textView = vulkanResourceManager->getTextureView(textureViewBinding.textureView);
        VulkanSampler *sampler = vulkanResourceManager->getSampler(textureViewBinding.sampler);
        assert(textView != nullptr);
        assert(sampler != nullptr);
        imageInfo.imageView = textView->imageView;
        imageInfo.sampler = sampler->sampler;

        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        break;
    }
    case ResourceBindingType::SampledImage: {
        const TextureViewBinding &textureViewBinding = entry.resource.textureViewBinding();
        VulkanTextureView *textView = vulkanResourceManager->getTextureView(textureViewBinding.textureView);
        assert(textView != nullptr);
        imageInfo.imageView = textView->imageView;

        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        break;
    }
    case ResourceBindingType::Sampler: {
        const SamplerBinding &samplerBinding = entry.resource.samplerBinding();
        VulkanSampler *sampler = vulkanResourceManager->getSampler(samplerBinding.sampler);
        assert(sampler != nullptr);
        imageInfo.sampler = sampler->sampler;

        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        break;
    }
    case ResourceBindingType::StorageImage: {
        const ImageBinding &imageBinding = entry.resource.imageBinding();
        VulkanTextureView *textView = vulkanResourceManager->getTextureView(imageBinding.textureView);
        assert(textView != nullptr);
        imageInfo.imageView = textView->imageView;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL; // Since we can read or write to these types of resources

        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        break;
    }
    case ResourceBindingType::UniformBuffer: {
        const UniformBufferBinding &bufferBinding = entry.resource.uniformBufferBinding();
        VulkanBuffer *buffer = vulkanResourceManager->getBuffer(bufferBinding.buffer);
        assert(buffer != nullptr);
        bufferInfo.buffer = buffer->buffer; // VkBuffer
        bufferInfo.offset = bufferBinding.offset;
        bufferInfo.range = (bufferBinding.size == UniformBufferBinding::WholeSize) ? VK_WHOLE_SIZE : bufferBinding.size;

        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        break;
    }
    case ResourceBindingType::StorageBuffer: {
        const StorageBufferBinding &bufferBinding = entry.resource.storageBufferBinding();
        VulkanBuffer *buffer = vulkanResourceManager->getBuffer(bufferBinding.buffer);
        assert(buffer != nullptr);
        bufferInfo.buffer = buffer->buffer; // VkBuffer
        bufferInfo.offset = bufferBinding.offset;
        bufferInfo.range = (bufferBinding.size == StorageBufferBinding::WholeSize) ? VK_WHOLE_SIZE : bufferBinding.size;

        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        break;
    }
    case ResourceBindingType::DynamicUniformBuffer: {
        const DynamicUniformBufferBinding &bufferBinding = entry.resource.dynamicUniformBufferBinding();
        VulkanBuffer *buffer = vulkanResourceManager->getBuffer(bufferBinding.buffer);
        assert(buffer != nullptr);
        bufferInfo.buffer = buffer->buffer; // VkBuffer
        bufferInfo.offset = bufferBinding.offset;
        bufferInfo.range = (bufferBinding.size == StorageBufferBinding::WholeSize) ? VK_WHOLE_SIZE : bufferBinding.size;

        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        break;
    }
    default:
        break;
    }

    if (descriptorWrite.descriptorCount > 0)
        vkUpdateDescriptorSets(vulkanDevice->device, 1, &descriptorWrite, 0, nullptr);
}

} // namespace KDGpu

/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/command_buffer.h>
#include <KDGpu/command_recorder.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/handle.h>

#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;

struct Device_t;
struct Buffer_t;

/**
 * @brief VulkanCommandRecorder
 * \ingroup vulkan
 *
 */
struct KDGPU_EXPORT VulkanCommandRecorder {
    explicit VulkanCommandRecorder(VkCommandPool _commandPool,
                                   const Handle<CommandBuffer_t> _commandBufferHandle,
                                   VulkanResourceManager *_vulkanResourceManager,
                                   const Handle<Device_t> &_deviceHandle);

    void begin();
    void blitTexture(const TextureBlitOptions &options);
    void clearBuffer(const BufferClear &clear);
    void clearColorTexture(const ClearColorTexture &clear);
    void clearDepthStencilTexture(const ClearDepthStencilTexture &clear);
    void copyBuffer(const BufferCopy &copy);
    void copyBufferToTexture(const BufferToTextureCopy &copy);
    void copyTextureToBuffer(const TextureToBufferCopy &copy);
    void copyTextureToTexture(const TextureToTextureCopy &copy);
    void updateBuffer(const BufferUpdate &update);
    void memoryBarrier(const MemoryBarrierOptions &options);
    void bufferMemoryBarrier(const BufferMemoryBarrierOptions &options);
    void textureMemoryBarrier(const TextureMemoryBarrierOptions &options);
    void executeSecondaryCommandBuffer(const Handle<CommandBuffer_t> &secondaryCommandBuffer);
    void resolveTexture(const TextureResolveOptions &options);
    void buildAccelerationStructures(const BuildAccelerationStructureOptions &options);
    void beginDebugLabel(const DebugLabelOptions &options);
    void endDebugLabel();

    Handle<CommandBuffer_t> finish();

    VkCommandPool commandPool{ VK_NULL_HANDLE };
    VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
    Handle<CommandBuffer_t> commandBufferHandle;
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
};

} // namespace KDGpu

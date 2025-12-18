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
                                   const Handle<CommandBuffer_t> &_commandBufferHandle,
                                   VulkanResourceManager *_vulkanResourceManager,
                                   const Handle<Device_t> &_deviceHandle);

    void begin() const;
    void blitTexture(const TextureBlitOptions &options) const;
    void clearBuffer(const BufferClear &clear) const;
    void clearColorTexture(const ClearColorTexture &clear) const;
    void clearDepthStencilTexture(const ClearDepthStencilTexture &clear) const;
    void copyBuffer(const BufferCopy &copy) const;
    void copyBufferToTexture(const BufferToTextureCopy &copy) const;
    void copyTextureToBuffer(const TextureToBufferCopy &copy) const;
    void copyTextureToTexture(const TextureToTextureCopy &copy) const;
    void updateBuffer(const BufferUpdate &update) const;
    void memoryBarrier(const MemoryBarrierOptions &options) const;
    void bufferMemoryBarrier(const BufferMemoryBarrierOptions &options) const;
    void textureMemoryBarrier(const TextureMemoryBarrierOptions &options) const;
    void executeSecondaryCommandBuffer(const Handle<CommandBuffer_t> &secondaryCommandBuffer) const;
    void resolveTexture(const TextureResolveOptions &options) const;
    void buildAccelerationStructures(const BuildAccelerationStructureOptions &options) const;
    void beginDebugLabel(const DebugLabelOptions &options) const;
    void endDebugLabel() const;
    [[nodiscard]] Handle<CommandBuffer_t> finish() const;

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    VkCommandPool commandPool{ VK_NULL_HANDLE };
    VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
    Handle<CommandBuffer_t> commandBufferHandle;
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

} // namespace KDGpu

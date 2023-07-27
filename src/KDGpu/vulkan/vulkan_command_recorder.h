/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/api/api_command_recorder.h>
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
struct KDGPU_EXPORT VulkanCommandRecorder : public ApiCommandRecorder {
    explicit VulkanCommandRecorder(VkCommandPool _commandPool,
                                   const Handle<CommandBuffer_t> _commandBufferHandle,
                                   VulkanResourceManager *_vulkanResourceManager,
                                   const Handle<Device_t> &_deviceHandle);

    void begin() final;
    void blitTexture(const TextureBlitOptions &options) final;
    void clearBuffer(const BufferClear &clear) final;
    void clearColorTexture(const ClearColorTexture &clear);
    void clearDepthStencilTexture(const ClearDepthStencilTexture &clear);
    void copyBuffer(const BufferCopy &copy) final;
    void copyBufferToTexture(const BufferToTextureCopy &copy) final;
    void copyTextureToBuffer(const TextureToBufferCopy &copy) final;
    void copyTextureToTexture(const TextureToTextureCopy &copy) final;
    void updateBuffer(const BufferUpdate &update) final;
    void memoryBarrier(const MemoryBarrierOptions &options) final;
    void bufferMemoryBarrier(const BufferMemoryBarrierOptions &options) final;
    void textureMemoryBarrier(const TextureMemoryBarrierOptions &options) final;
    void executeSecondaryCommandBuffer(const Handle<CommandBuffer_t> &secondaryCommandBuffer) final;
    void resolveTexture(const TextureResolveOptions &options) final;
    Handle<CommandBuffer_t> finish() final;

    VkCommandPool commandPool{ VK_NULL_HANDLE };
    VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
    Handle<CommandBuffer_t> commandBufferHandle;
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
};

} // namespace KDGpu

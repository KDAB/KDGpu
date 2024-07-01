/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "render_pass.h"

#include <KDGpu/graphics_api.h>
#include <KDGpu/resource_manager.h>

namespace KDGpu {

/**
    @class RenderPass
    @brief RenderPass is a representation of a rendering instance
    @ingroup public
    @headerfile render_pass.h <KDGpu/render_pass.h>

    Renderpass defines:
    - a list of rendering attachments
    - a list of subpasses that reference attachments and what they will be used for
    - a list of dependencies between the different subpasses
    - Optionally, a list of multiview masks used throughout the subpasses if multiview is enabled

    In return, the RenderPass tells the driver how resources will be accessed by the difference subpasses
    so that it can optimize memory accesses and synchronizations to the attachments which is especially imported on Tile based GPUs.

    RenderPass instances are provided by the logical Device. The RenderPass is used to instruct the CommandRecorder the rendering architectures.

    Also, GraphicsPipelines are to be bound against a RenderPass at a specific subpass index. They can be reused across multiple compatible RenderPasses
    (same attachment count and format). The renderpass ultimately being used is the one specified by the CommandRecorder.

    @sa KDGpu::Device::createRenderPass
    @sa KDGpu::GraphicsPipeline
    @sa KDGpu::CommandRecorder
 */

/**
    @fn RenderPass::handle()
    @brief Returns the handle used to retrieve the underlying API specific RenderPass

    @sa ResourceManager
 */

/**
    @fn RenderPass::isValid()
    @brief Convenience function to check whether the RenderPass is actually referencing a valid API specific resource
 */

} // namespace KDGpu

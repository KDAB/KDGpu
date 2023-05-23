/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>

#include <KDGpu/kdgpu_export.h>

namespace KDGpu {

class GraphicsApi;

struct Device_t;
struct GraphicsPipeline_t;
struct GraphicsPipelineOptions;

/**
 * @brief GraphicsPipeline
 * @ingroup public
 */
class KDGPU_EXPORT GraphicsPipeline
{
public:
    GraphicsPipeline();
    ~GraphicsPipeline();

    GraphicsPipeline(GraphicsPipeline &&);
    GraphicsPipeline &operator=(GraphicsPipeline &&);

    GraphicsPipeline(const GraphicsPipeline &) = delete;
    GraphicsPipeline &operator=(const GraphicsPipeline &) = delete;

    const Handle<GraphicsPipeline_t> &handle() const noexcept { return m_graphicsPipeline; }
    bool isValid() const noexcept { return m_graphicsPipeline.isValid(); }

    operator Handle<GraphicsPipeline_t>() const noexcept { return m_graphicsPipeline; }

private:
    explicit GraphicsPipeline(GraphicsApi *api, const Handle<Device_t> &device, const GraphicsPipelineOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<GraphicsPipeline_t> m_graphicsPipeline;

    friend class Device;
    friend KDGPU_EXPORT bool operator==(const GraphicsPipeline &, const GraphicsPipeline &);
};

KDGPU_EXPORT bool operator==(const GraphicsPipeline &a, const GraphicsPipeline &b);
KDGPU_EXPORT bool operator!=(const GraphicsPipeline &a, const GraphicsPipeline &b);

} // namespace KDGpu

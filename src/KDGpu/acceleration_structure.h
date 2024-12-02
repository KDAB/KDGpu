/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/graphics_api.h>

namespace KDGpu {

struct Device_t;
struct AccelerationStructure_t;
struct BufferOptions;
struct AccelerationStructureOptions;

/**
 * @brief AccelerationStructure
 * @ingroup public
 */
class KDGPU_EXPORT AccelerationStructure
{
public:
    ~AccelerationStructure();
    AccelerationStructure();

    AccelerationStructure(AccelerationStructure &&) noexcept;
    AccelerationStructure &operator=(AccelerationStructure &&) noexcept;

    AccelerationStructure(const AccelerationStructure &) = delete;
    AccelerationStructure &operator=(const AccelerationStructure &) = delete;

    const Handle<AccelerationStructure_t> &handle() const noexcept { return m_accelerationStructure; }
    bool isValid() const noexcept { return m_accelerationStructure.isValid(); }

    operator Handle<AccelerationStructure_t>() const noexcept { return m_accelerationStructure; }

private:
    explicit AccelerationStructure(GraphicsApi *api, const Handle<Device_t> &device, const AccelerationStructureOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<AccelerationStructure_t> m_accelerationStructure;

    friend class Device;
    friend class Queue;
    friend KDGPU_EXPORT bool operator==(const AccelerationStructure &, const AccelerationStructure &);
};

KDGPU_EXPORT bool operator==(const AccelerationStructure &a, const AccelerationStructure &b);
KDGPU_EXPORT bool operator!=(const AccelerationStructure &a, const AccelerationStructure &b);

} // namespace KDGpu

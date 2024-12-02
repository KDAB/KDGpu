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
struct Sampler_t;
struct SamplerOptions;

/**
 * @brief Sampler
 * @ingroup public
 */
class KDGPU_EXPORT Sampler
{
public:
    Sampler();
    ~Sampler();

    Sampler(Sampler &&) noexcept;
    Sampler &operator=(Sampler &&) noexcept;

    Sampler(const Sampler &) = delete;
    Sampler &operator=(const Sampler &) = delete;

    Handle<Sampler_t> handle() const noexcept { return m_sampler; }
    bool isValid() const noexcept { return m_sampler.isValid(); }

    operator Handle<Sampler_t>() const noexcept { return m_sampler; }

private:
    Sampler(GraphicsApi *api, const Handle<Device_t> &device, const SamplerOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<Sampler_t> m_sampler;

    friend class Device;
    friend KDGPU_EXPORT bool operator==(const Sampler &, const Sampler &);
};

KDGPU_EXPORT bool operator==(const Sampler &a, const Sampler &b);
KDGPU_EXPORT bool operator!=(const Sampler &a, const Sampler &b);

} // namespace KDGpu

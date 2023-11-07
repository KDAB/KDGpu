/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>
#include <KDGpu/handle.h>
#include <KDGpu/surface_options.h>
#include <KDGpu/adapter.h>

#include <vector>

namespace KDGpu {

struct Instance_t;
struct Surface_t;

/**
 * @brief ApiInstance
 * \ingroup api
 *
 */
struct ApiInstance {
    virtual std::vector<Extension> extensions() const = 0;
    virtual std::vector<Handle<Adapter_t>> queryAdapters(const Handle<Instance_t> &instanceHandle) = 0;
    virtual std::vector<AdapterGroup> queryAdapterGroups() = 0;
    virtual Handle<Surface_t> createSurface(const SurfaceOptions &options) = 0;
};

} // namespace KDGpu

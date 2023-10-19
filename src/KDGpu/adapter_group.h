/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <vector>

namespace KDGpu {

struct Adapter_t;

struct AdapterGroup {
    std::vector<Handle<Adapter_t>> adapters;
    bool supportsSubsetAllocations{ false };
};

} // namespace KDGpu

/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/kdxr_core.h>
#include <KDGpu/handle.h>

namespace KDXr {

struct LocateSpaceOptions;
struct ReferenceSpace_t;

/**
 * @brief ApiReferenceSpace
 * \ingroup api
 *
 */
struct ApiReferenceSpace {
    virtual LocateSpaceResult locateSpace(const LocateSpaceOptions &options, SpaceState &state) = 0;
};

} // namespace KDXr

/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "graphics_api.h"

namespace KDGpu {

GraphicsApi::GraphicsApi()
{
}

GraphicsApi::~GraphicsApi()
{
}

Instance GraphicsApi::createInstance(const InstanceOptions &options)
{
    return Instance(this, options);
}

} // namespace KDGpu

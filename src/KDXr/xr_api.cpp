/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "xr_api.h"

namespace KDXr {

XrApi::XrApi()
{
}

XrApi::~XrApi()
{
}

Instance XrApi::createInstance(const InstanceOptions &options)
{
    return Instance(this, options);
}

} // namespace KDXr

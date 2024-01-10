/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "openxr_enums.h"

namespace KDXr {

XrFormFactor formFactorToXrFormFactor(FormFactor formFactor)
{
    return static_cast<XrFormFactor>(formFactor);
}

} // namespace KDXr

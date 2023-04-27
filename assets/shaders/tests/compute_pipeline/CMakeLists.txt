# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
CompileShader(KDGpu_EmptyCompute empty_compute.comp empty_compute.comp.spv)
add_custom_target(
    KDGpu_EmptyComputeTmp ALL
    DEPENDS KDGpu_EmptyCompute
    COMMENT "Target to compile the empty shader"
)

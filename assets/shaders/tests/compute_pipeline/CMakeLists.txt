# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
kdgpu_compileshader(KDGpu_EmptyCompute empty_compute.comp empty_compute.comp.spv)
add_custom_target(
    KDGpu_EmptyComputeTmp ALL
    DEPENDS KDGpu_EmptyCompute
    COMMENT "Target to compile the empty shader"
)
kdgpu_compileshader(
    KDGpu_EmptyComputeSpecializationConstant specialization_constant.comp specialization_constant.comp.spv
)
add_custom_target(
    KDGpu_EmptyComputeSpecializationConstantsTmp ALL
    DEPENDS KDGpu_EmptyComputeSpecializationConstant
    COMMENT "Target to compile the empty shader"
)

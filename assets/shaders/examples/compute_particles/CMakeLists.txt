# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
kdgpu_compileshaderset(KDGpu_ComputeParticles triangle)

kdgpu_compileshader(KDGpu_ComputeParticles particles.comp particles.comp.spv)
add_custom_target(
    KDGpu_ComputeParticleTmp ALL
    DEPENDS KDGpu_ComputeParticles
    COMMENT "Target to compile the particle shader"
)

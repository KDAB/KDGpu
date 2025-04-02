# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

kdgpu_compileshader(KDGpu_ComputeOITCompParticlesShader particles.comp particles.comp.spv)
kdgpu_compileshader(KDGpu_ComputeOITSphereShader sphere_instanced.vert sphere_instanced.vert.spv)
kdgpu_compileshader(KDGpu_ComputeOITCubeShader cube.vert cube.vert.spv)
kdgpu_compileshader(KDGpu_ComputeOITAlphaShader alpha.frag alpha.frag.spv)
kdgpu_compileshaderset(KDGpu_ComputeOITCompositing compositing)

add_custom_target(
    KDGpu_ComputeOITShaders ALL
    DEPENDS KDGpu_ComputeOITCompParticlesShader
            KDGpu_ComputeOITSphereShader
            KDGpu_ComputeOITCubeShader
            KDGpu_ComputeOITAlphaShader
            KDGpu_ComputeOITCompositingShaders
    COMMENT "Target to compile the compute_oit_transparency shaders"
)

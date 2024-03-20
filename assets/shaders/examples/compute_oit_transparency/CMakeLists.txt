# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

CompileShader(KDGpu_ComputeOITCompParticlesShader particles.comp particles.comp.spv)
CompileShader(KDGpu_ComputeOITSphereShader sphere_instanced.vert sphere_instanced.vert.spv)
CompileShader(KDGpu_ComputeOITCubeShader cube.vert cube.vert.spv)
CompileShader(KDGpu_ComputeOITAlphaShader alpha.frag alpha.frag.spv)
CompileShaderSet(KDGpu_ComputeOITCompositing compositing)

add_custom_target(
    KDGpu_ComputeOITShaders ALL
    DEPENDS KDGpu_ComputeOITCompParticlesShader
            KDGpu_ComputeOITSphereShader
            KDGpu_ComputeOITCubeShader
            KDGpu_ComputeOITAlphaShader
            KDGpu_ComputeOITCompositingShaders
    COMMENT "Target to compile the compute_oit_transparency shaders"
)

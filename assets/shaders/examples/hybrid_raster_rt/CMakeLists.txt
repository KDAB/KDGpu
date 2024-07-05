# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

CompileShader(KDGpu_HybridRasterRtCompParticlesShader particles.comp particles.comp.spv)
CompileShader(KDGpu_HybridRasterRtSphereShader sphere_instanced.vert sphere_instanced.vert.spv)
CompileShader(KDGpu_HybridRasterRtPlaneShader plane.vert plane.vert.spv)
CompileShader(KDGpu_HybridRasterRtOpaqueShader opaque.frag opaque.frag.spv)
CompileShader(KDGpu_HybridRasterRtZFillShader zfill.frag zfill.frag.spv)
CompileShader(KDGpu_HybridRasterRtAlphaShader alpha.frag alpha.frag.spv)
CompileShaderSet(KDGpu_HybridRasterRtLight light)
CompileShaderSet(KDGpu_HybridRasterRtCompositing compositing)
CompileRTShader(KDGpu_HybridRasterRtShadowRtGenShader shadow.rgen shadow.rgen.spv)
CompileRTShader(KDGpu_HybridRasterRtShadowRtMissShader shadow.rmiss shadow.rmiss.spv)
CompileRTShader(KDGpu_HybridRasterRtShadowRtAHitShader shadow.rahit shadow.rahit.spv)
CompileRTShader(KDGpu_HybridRasterRtShadowRtCHitShader shadow.rchit shadow.rchit.spv)

add_custom_target(
    KDGpu_HybridRasterRtShaders ALL
    DEPENDS KDGpu_HybridRasterRtCompParticlesShader
            KDGpu_HybridRasterRtSphereShader
            KDGpu_HybridRasterRtPlaneShader
            KDGpu_HybridRasterRtZFillShader
            KDGpu_HybridRasterRtAlphaShader
            KDGpu_HybridRasterRtOpaqueShader
            KDGpu_HybridRasterRtCompositingShaders
            KDGpu_HybridRasterRtLightShaders
            KDGpu_HybridRasterRtShadowRtGenShader
            KDGpu_HybridRasterRtShadowRtMissShader
            KDGpu_HybridRasterRtShadowRtAHitShader
            KDGpu_HybridRasterRtShadowRtCHitShader
    COMMENT "Target to compile the hybrid_raster_rt shaders"
)

# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

kdgpu_compileshader(KDGpu_HybridRasterRtCompParticlesShader particles.comp particles.comp.spv)
kdgpu_compileshader(KDGpu_HybridRasterRtSphereShader sphere_instanced.vert sphere_instanced.vert.spv)
kdgpu_compileshader(KDGpu_HybridRasterRtPlaneShader plane.vert plane.vert.spv)
kdgpu_compileshader(KDGpu_HybridRasterRtOpaqueShader opaque.frag opaque.frag.spv)
kdgpu_compileshader(KDGpu_HybridRasterRtZFillShader zfill.frag zfill.frag.spv)
kdgpu_compileshader(KDGpu_HybridRasterRtAlphaShader alpha.frag alpha.frag.spv)
kdgpu_compileshaderset(KDGpu_HybridRasterRtLight light)
kdgpu_compileshaderset(KDGpu_HybridRasterRtCompositing compositing)
kdgpu_compilertshader(KDGpu_HybridRasterRtShadowRtGenShader shadow.rgen shadow.rgen.spv)
kdgpu_compilertshader(KDGpu_HybridRasterRtShadowRtMissShader shadow.rmiss shadow.rmiss.spv)
kdgpu_compilertshader(KDGpu_HybridRasterRtShadowRtAHitShader shadow.rahit shadow.rahit.spv)
kdgpu_compilertshader(KDGpu_HybridRasterRtShadowRtCHitShader shadow.rchit shadow.rchit.spv)

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

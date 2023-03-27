CompileShaderSet(KDGpu_ComputeParticles triangle)

CompileShader(KDGpu_ComputeParticles particles.comp particles.comp.spv)
add_custom_target(KDGpu_ComputeParticleTmp ALL
    DEPENDS KDGpu_ComputeParticles
)

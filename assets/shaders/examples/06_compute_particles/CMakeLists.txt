CompileShaderSet(ToyRendererComputeParticles triangle)

CompileShader(ToyRendererComputeParticles particles.comp particles.comp.spv)
add_custom_target(ToyRendererComputeParticleTmp ALL
    DEPENDS ToyRendererComputeParticles
)

CompileShader(ToyRendererEmptyCompute empty_compute.comp empty_compute.comp.spv)
add_custom_target(ToyRendererEmptyComputeTmp ALL
    DEPENDS ToyRendererEmptyCompute
)

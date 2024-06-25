#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_buffer_reference : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_ARB_gpu_shader_int64 : enable

layout(location = 0) in vec3 vertexPosition;

layout(location = 0) out vec3 color;

// The reference to our buffer, aligned to 16 bytes (vec4)
layout(buffer_reference, scalar, buffer_reference_align = 16) buffer VertexColors
{
    vec4 colors[];
};

// Push constant holding the address of our VertexColors buffer
layout(push_constant) uniform BufferReferences
{
    uint64_t vertexColors;
}
bufferReferences;

void main()
{
    VertexColors vColors = VertexColors(bufferReferences.vertexColors); // Retrieve Buffer from its address
    color = vColors.colors[gl_VertexIndex].rgb;

    gl_Position = vec4(vertexPosition, 1.0);
}

#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 color;

layout(location = 0) out vec4 fragColor;

layout (constant_id = 2) const int THIRD_CONSTANT = 64;


void main()
{
    if (THIRD_CONSTANT == 8)
        discard;
    fragColor = vec4(color, 1.0);
}

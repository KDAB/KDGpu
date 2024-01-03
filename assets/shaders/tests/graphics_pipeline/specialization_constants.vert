#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 vertexPosition;

layout(location = 0) out vec3 color;

layout (constant_id = 0) const int FIRST_CONSTANT = 64;
layout (constant_id = 1) const int SECOND_CONSTANT = 64;

void main()
{
    color = vec3(0.0);
    if (FIRST_CONSTANT == 16)
        color.r = 1.0;
    if (SECOND_CONSTANT == 32)
        color.g = 1.0;

    gl_Position = vec4(vertexPosition, 1.0);
}

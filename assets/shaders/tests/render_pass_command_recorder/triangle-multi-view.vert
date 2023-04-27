#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_multiview : enable

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;

layout(location = 0) out vec3 color;

const float xOffsets[2] = { -0.5, 0.5 };

void main()
{
    color = vertexColor;
    gl_Position = vec4(vertexPosition + vec3(xOffsets[gl_ViewIndex], 0.0, 0.0) , 1.0);
}

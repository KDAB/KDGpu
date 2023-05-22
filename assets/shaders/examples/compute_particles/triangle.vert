#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec4 particlePosition;
layout(location = 2) in vec4 particleColor;

layout(location = 0) out vec3 color;

void main()
{
    color = particleColor.rgb;
    gl_Position = vec4(vertexPosition + particlePosition.xyz, 1.0);
}

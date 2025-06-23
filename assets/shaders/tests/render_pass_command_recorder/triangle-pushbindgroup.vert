#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 vertexPosition;

layout(location = 0) out vec2 tCoords;

layout(set = 0, binding = 0) uniform Mats
{
    mat4 mvp;
}
mats;

void main()
{
    tCoords = vertexPosition.xy;
    gl_Position = mats.mvp * vec4(vertexPosition, 1.0);
}

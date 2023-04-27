#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec3 position;
layout(location = 1) out vec2 texCoord;

vec3 vertices[6] =
{
    vec3(-1.0, -1.0, 0.5),
    vec3(-1.0, 1.0, 0.5),
    vec3(1.0, 1.0, 0.5),
    vec3(1.0, 1.0, 0.5),
    vec3(1.0, -1.0, 0.5),
    vec3(-1.0, -1.0, 0.5),
};

vec2 texCoords[6] = {
    vec2(0.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, 0.0),
    vec2(0.0, 0.0)
};

void main()
{
    texCoord = texCoords[gl_VertexIndex];
    gl_Position = vec4(vertices[gl_VertexIndex], 1.0);
}

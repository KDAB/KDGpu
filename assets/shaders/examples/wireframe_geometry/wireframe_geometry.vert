#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in float vertexExcludeEdge;

layout(location = 0) out vec3 position;
layout(location = 1) out vec3 normal;
layout(location = 2) out float excludeEdge;

layout(set = 0, binding = 0) uniform Camera
{
    mat4 view;
    mat4 projection;
}
camera;

layout(set = 2, binding = 0) uniform Entity
{
    mat4 model;
}
entity;

void main()
{
    excludeEdge = vertexExcludeEdge;
    normal = normalize((camera.view * entity.model * vec4(vertexNormal, 0.0)).xyz);
    gl_Position = camera.projection * camera.view * entity.model * vec4(vertexPosition, 1.0);
}

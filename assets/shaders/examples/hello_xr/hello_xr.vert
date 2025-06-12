#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;

layout(location = 0) out vec3 color;

layout(set = 0, binding = 0) uniform Camera
{
    mat4 viewProjection;
}
camera;

layout(set = 1, binding = 0) uniform Entity
{
    mat4 modelMatrix;
}
entity;

void main()
{
    color = vertexColor;
    gl_Position = camera.viewProjection * entity.modelMatrix * vec4(vertexPosition, 1.0);
}

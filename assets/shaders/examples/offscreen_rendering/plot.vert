#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 vertexPos;
layout(location = 1) in vec4 vertexCol;

layout(location = 0) out vec4 color;

out gl_PerVertex
{
    vec4 gl_Position;
    float gl_PointSize;
};

layout(set = 1, binding = 0) uniform Transform
{
    mat4 proj;
}
transform;

void main()
{
    color = vertexCol;
    gl_PointSize = 16.0;
    gl_Position = transform.proj * vertexPos;
}

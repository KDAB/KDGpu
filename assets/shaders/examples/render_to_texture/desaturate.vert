#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexTexCoord;

layout(location = 0) out vec3 position;
layout(location = 1) out vec2 texCoord;

void main()
{
    texCoord = vertexTexCoord;
    gl_Position = vec4(vertexPosition, 1.0);
}

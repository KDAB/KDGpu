#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;

layout(location = 0) out vec3 color;

layout(push_constant) uniform PushConstants {
    mat4 modelMatrix;
} pushConstants;

void main()
{
    color = vertexColor;
    gl_Position = pushConstants.modelMatrix * vec4(vertexPosition, 1.0);
}

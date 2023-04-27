#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_multiview : enable

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;

layout(location = 0) out vec3 color;

layout(push_constant) uniform PushConstants {
    float angle;
} pushConstants;

mat3 rotationAroundZ(float angle)
{
    float s = sin(angle);
    float c = cos(angle);

  return mat3(c, s, 0.0,
             -s, c, 0.0,
             0.0, 0.0, 1.0);
}

void main()
{
    color = vertexColor;
    float rotationSign = gl_ViewIndex == 0 ? -1.0 : 1.0;
    gl_Position = vec4(vertexPosition * rotationAroundZ(rotationSign * pushConstants.angle) , 1.0);
}

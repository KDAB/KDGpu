#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) uniform sampler2DArray colorTexture;

layout(push_constant) uniform PushConstants {
    int arrayLayer;
} pushConstants;

void main()
{
    vec3 color = texture(colorTexture, vec3(texCoord, pushConstants.arrayLayer)).rgb;
    fragColor = vec4(color, 1.0);
}

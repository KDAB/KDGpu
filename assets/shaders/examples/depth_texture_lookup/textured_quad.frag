#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) uniform sampler2D depthTexture;

void main()
{
    float depth = texture(depthTexture, texCoord).r;
    fragColor = vec4(vec3(depth), 1.0);
}

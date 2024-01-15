#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 color;

layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) uniform sampler2D colorTexture;

layout(push_constant) uniform PushConstants
{
    layout(offset = 64) vec2 viewportSize;
    bool useTexture;
}
fragPushConstants;

void main()
{
    fragColor = fragPushConstants.useTexture ? texture(colorTexture, gl_FragCoord.xy / fragPushConstants.viewportSize) : vec4(color, 1.0);
}

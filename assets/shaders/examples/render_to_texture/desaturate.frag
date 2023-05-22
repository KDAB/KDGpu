#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) uniform sampler2D colorTexture;

layout(push_constant) uniform PushConstants {
    float filterPosition;
} pushConstants;


float luminance(vec3 color)
{
    return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
}

void main()
{
    vec3 color = texture(colorTexture, texCoord).rgb;

    const float lineWidth = 0.001;
    if (texCoord.s > pushConstants.filterPosition + lineWidth) {
        float gray = luminance(texture(colorTexture, texCoord).rgb);
        fragColor = vec4(gray, gray, gray, 1.0);
    } else if (texCoord.s < pushConstants.filterPosition - lineWidth) {
        fragColor = vec4(color, 1.0);
    } else {
        fragColor = vec4( 1.0, 0.0, 0.0, 1.0 );
    }
}

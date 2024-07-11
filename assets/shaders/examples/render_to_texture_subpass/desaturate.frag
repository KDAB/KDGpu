#version 450

layout(location = 0) in vec2 texCoord;
layout(input_attachment_index = 0, binding = 0) uniform subpassInput inputColor;

layout(location = 0) out vec4 fragColor;

layout(push_constant) uniform PushConstants {
    float filterPosition;
} pushConstants;


float luminance(vec3 color)
{
    return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
}

void main()
{
    vec3 color = subpassLoad(inputColor).rgb;

    const float lineWidth = 0.001;
    if (texCoord.s > pushConstants.filterPosition + lineWidth) {
        float gray = luminance(color);
        fragColor = vec4(gray, gray, gray, 1.0);
    } else if (texCoord.s < pushConstants.filterPosition - lineWidth) {
        fragColor = vec4(color, 1.0);
    } else {
        fragColor = vec4( 0.0, 0.0, 1.0, 1.0 );
    }
}

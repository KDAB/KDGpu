#version 450

layout(input_attachment_index = 0, binding = 0) uniform subpassInput inputColor;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(subpassLoad(inputColor).rgb, 0);
}

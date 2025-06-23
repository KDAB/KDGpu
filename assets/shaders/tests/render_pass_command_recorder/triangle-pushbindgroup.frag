#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 tCoords;

layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 1) uniform sampler2D colorTexture;

void main()
{
    fragColor = texture(colorTexture, tCoords);
}

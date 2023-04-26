#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec4 color;

layout (location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) uniform sampler2D pointTexture;

void main ()
{
	fragColor = color * texture(pointTexture, gl_PointCoord);
}

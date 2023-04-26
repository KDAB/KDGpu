#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) uniform sampler2D pointTexture;

void main ()
{
	vec4 color = vec4(0.133, 0.773, 0.369, 1.0);
	fragColor = color * texture(pointTexture, gl_PointCoord);
}

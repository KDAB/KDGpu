#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;

void main()
{
    // Hard-wired gray-200 color for now
    fragColor = vec4(0.898, 0.906, 0.922, 1.0);
}

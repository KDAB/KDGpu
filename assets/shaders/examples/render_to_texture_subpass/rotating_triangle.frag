#version 450

//![scene_frag]
layout(location = 0) in vec3 color;

layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = vec4(color, 1.0);
}
//![scene_frag]

#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec2 texCoord;

const vec3 corners[4] = vec3[4](
    vec3(-1.0f, 1.0, 0.0), // Top Left
    vec3(-1.0f, -1.0, 0.0), // Bottom Left
    vec3(1.0f, 1.0, 0.0), // Top Right
    vec3(1.0f, -1.0, 0.0) // Bottom Right
);

const int indices[6] = int[6](
    3, 1, 0,
    0, 2, 3
);

void main()
{
    vec3 pos = corners[indices[gl_VertexIndex]];

    texCoord = (1.0 + pos.xy) * 0.5;
    gl_Position = vec4(pos * 0.25 - vec3(0.75, 0.75, 0.0), 1.0);
}

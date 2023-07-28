#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(push_constant) uniform PushConstants {
    mat4 rotation;
} pushConstants;

const vec3 cubeCorners[8] = vec3[8](
    // Top Left Front
    vec3(-0.5f, 0.5, 0.5),
    // Top Left Rear
    vec3(-0.5f, 0.5, -0.5),

    // Bottom Left Front
    vec3(-0.5f, -0.5, 0.5),
     // Bottom Left Rear
    vec3(-0.5f, -0.5, -0.5),

    // Top Right Front
    vec3(0.5f, 0.5, 0.5),
    // Top Right Rear
    vec3(0.5f, 0.5, -0.5),

    // Bottom Right Front
    vec3(0.5f, -0.5, 0.5),
     // Bottom Right Rear
    vec3(0.5f, -0.5, -0.5)
);

const int indices[36] = int[36](
    // Top
    2, 1, 0,
    2, 3, 1,

    // Front
    6, 0, 4,
    0, 6, 2,

    // Left,
    0, 1, 5,
    5, 4, 0,

    // Right
    2, 6, 7,
    7, 3, 2,

    // Rear
    1, 7, 5,
    3, 7, 1,

    // Botton
    4, 5, 6,
    6, 5, 7

);

void main()
{
    gl_Position = pushConstants.rotation * vec4(cubeCorners[indices[gl_VertexIndex]], 1.0);
}

#version 450
#extension GL_ARB_separate_shader_objects : enable

vec3 vertices[8] = {
    vec3(-1.0, 0.0, 0.0),
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, -1.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, -1.0),
    vec3(0.0, 0.0, 1.0),
    vec3(0.0, 0.0, 0.0),
    vec3(0.0, 0.0, 0.0),
};

layout(set = 0, binding = 0) uniform Camera
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
}
camera;

layout(push_constant) uniform PushConstants
{
    vec3 lightWorldPos;
}
pushConstants;

void main()
{
    vec3 vPos = pushConstants.lightWorldPos + vertices[gl_VertexIndex] * 5.0;

    if (gl_VertexIndex == 7) {
        vec3 lightDir = normalize(-pushConstants.lightWorldPos);
        vPos = pushConstants.lightWorldPos + lightDir * 30.0;
    }

    gl_Position = camera.projectionMatrix * camera.viewMatrix * vec4(vPos, 1.0);
}

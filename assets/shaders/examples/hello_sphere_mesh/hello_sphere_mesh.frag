#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec3 worldNormal;

void main()
{
    // Compute some lighting because we can
    vec3 lightDir = normalize(vec3(1.0));

    // Diffuse Factor
    float diffuse = max(dot(lightDir, normalize(worldNormal)), 0.0);

    fragColor = vec4(vec3(0.8) * diffuse, 1.0);
}

#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outPos;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outColor;

layout(location = 0) in vec4 color;
layout(location = 1) in vec3 worldNormal;
layout(location = 2) in vec3 worldPosition;
layout(location = 3) in vec3 worldView;

layout(push_constant) uniform PushConstants
{
    vec3 lightWorldPos;
}
pushConstants;

vec4 adsModel(vec4 color)
{
    const float shininess = 50.0;

    // We perform all work in world space
    vec3 n = normalize(worldNormal);
    vec3 lightDir = -pushConstants.lightWorldPos;
    vec3 s = normalize(-lightDir);
    float sDotN = dot(s, n);

    // Calculate the diffuse factor
    float diffuse = max(sDotN, 0.0);

    // Calculate the specular factor
    float specular = 0.0;
    if (diffuse > 0.0 && shininess > 0.0) {
        float normFactor = (shininess + 2.0) / 2.0;
        vec3 r = reflect(s, n); // Reflection direction in world space
        specular = normFactor * pow(max(dot(r, worldView), 0.0), shininess);
    }

    // Accumulate the diffuse and specular contributions
    const vec3 lightDiffuseColor = vec3(1.0);
    const vec3 lightSpecularColor = vec3(0.2);
    vec3 diffuseFactor = diffuse * lightDiffuseColor;
    vec3 specularFactor = specular * lightSpecularColor;

    vec3 ambientColor = color.rgb * 0.4;
    const vec3 specularColor = vec3(1.0);
    vec4 lightedColor = vec4(ambientColor + diffuseFactor * color.rgb + specularFactor * specularColor, color.a);
    return lightedColor;
}

void main()
{
    outColor = adsModel(color);
    outNormal = vec4(normalize(worldNormal), 0);
    outPos = vec4(worldPosition, 1.0);
}

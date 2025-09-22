#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) noperspective in vec3 edgeDistance; // From geometry shader

layout(location = 0) out vec4 fragColor;

layout(set = 1, binding = 0) uniform Material
{
    vec4 baseColorFactor;
    vec4 wireframeColorAndWidth; // .xyz = color, .w = width in pixels
}
material;

// Some hardcoded lighting
const vec3 lightDir = vec3(1.0, 0.5, -0.5);
const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const vec3 ambientColor = vec3(0.1, 0.1, 0.1);

void main()
{
    vec4 baseColor = material.baseColorFactor;

    // An extremely simple directional lighting model, just to give our model some shape.
    vec3 N = normalize(normal);
    vec3 L = normalize(lightDir);
    float NDotL = max(dot(N, L), 0.0);
    vec3 surfaceColor = (baseColor.rgb * ambientColor) + (baseColor.rgb * vec3(NDotL));

    // Compute the distance to the nearest edge in pixels
    float edgeDist = min(min(edgeDistance.x, edgeDistance.y), edgeDistance.z);
    float edgeWidth = material.wireframeColorAndWidth.w; // in pixels

    // Compute the anti-aliased line factor using smoothstep
    float lineFactor = smoothstep(0.0, edgeWidth, edgeDist);

    // Mix the surface color with the wireframe color based on the line factor
    surfaceColor = mix(material.wireframeColorAndWidth.xyz, surfaceColor, lineFactor);

    fragColor = vec4(surfaceColor, baseColor.a);
}

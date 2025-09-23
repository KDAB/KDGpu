#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) noperspective in vec4 edgeA; // From geometry shader
layout(location = 3) noperspective in vec4 edgeB; // From geometry shader
layout(location = 4) flat in vec3 excludeEdge; // From geometry shader
layout(location = 5) flat in int edgeCase; // From geometry shader

layout(location = 0) out vec4 fragColor;

layout(set = 1, binding = 0) uniform Material
{
    vec4 baseColorFactor;
    vec4 wireframeColorAndWidth; // .xyz = color, .w = width in pixels
}
material;

// For excluded edges, we set a very high edge distance so that
// the fragment shader will never draw them.
const float maxEdgeWidth = 100.0;

// Some hardcoded lighting
const vec3 lightDir = vec3(1.0, 0.5, -0.5);
const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const vec3 ambientColor = vec3(0.5, 0.5, 0.5);

void main()
{
    vec4 baseColor = material.baseColorFactor;

    // An extremely simple directional lighting model, just to give our model some shape.
    vec3 N = normalize(normal);
    vec3 L = normalize(lightDir);
    float NDotL = max(dot(N, L), 0.0);
    vec3 surfaceColor = (baseColor.rgb * ambientColor) + (baseColor.rgb * vec3(NDotL));

    float edgeDistance = 0.0;

    if (edgeCase == 0) {
        // Common case where all vertices are within the viewport
        // Compute the distance to the nearest edge in pixels
        edgeDistance = min(min(edgeA.x, edgeA.y), edgeA.z);
    } else {
        // Handle case where screen space projection breaks down
        // Compute and compare the squared distances
        vec2 AF = gl_FragCoord.xy - edgeA.xy;
        float sqAF = dot(AF, AF);
        float AFcosA = dot(AF, edgeA.zw);
        float edgeDistanceA = max(abs(sqAF - AFcosA * AFcosA), excludeEdge.x * maxEdgeWidth * maxEdgeWidth);

        vec2 BF = gl_FragCoord.xy - edgeB.xy;
        float sqBF = dot(BF, BF);
        float BFcosB = dot(BF, edgeB.zw);
        float edgeDistanceB = max(abs(sqBF - BFcosB * BFcosB), excludeEdge.y * maxEdgeWidth * maxEdgeWidth);

        edgeDistance = min(edgeDistanceA, edgeDistanceB);

        // Only need to care about the 3rd edge for some cases.
        if (edgeCase == 1 || edgeCase == 2 || edgeCase == 4) {
            float AFcosA0 = dot(AF, normalize(edgeB.xy - edgeA.xy));
            float edgeDistanceC = max(abs(sqAF - AFcosA0 * AFcosA0), excludeEdge.z * maxEdgeWidth * maxEdgeWidth);
            edgeDistance = min(edgeDistance, edgeDistanceC);
        }

        edgeDistance = sqrt(edgeDistance);
    }

    // Compute the anti-aliased line factor using smoothstep
    float edgeWidth = material.wireframeColorAndWidth.w; // in pixels
    float lineFactor = smoothstep(0.0, edgeWidth, edgeDistance);

    // Mix the surface color with the wireframe color based on the line factor
    surfaceColor = mix(material.wireframeColorAndWidth.xyz, surfaceColor, lineFactor);

    fragColor = vec4(surfaceColor, baseColor.a);
}

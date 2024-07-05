#version 450
#extension GL_ARB_separate_shader_objects : enable

// Structs
struct AlphaFragment {
    vec4 color;
    float depth;
    uint next;
    float _pad[2];
};

layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) uniform sampler2D gBufferWorldPositions;
layout(set = 0, binding = 1) uniform sampler2D gBufferWorldNormals;
layout(set = 0, binding = 2) uniform sampler2D gBufferColors;
layout(set = 0, binding = 3) uniform sampler2D gBufferDepth;

layout(std430, set = 1, binding = 0) coherent readonly buffer AlphaFragments
{
    uint count;
    vec3 _pad;
    AlphaFragment fragments[];
}
alphaFragments;

layout(set = 1, binding = 1, r32ui) uniform readonly uimage2D alphaHeadPointerTexture;

layout(set = 2, binding = 0, r32f) uniform readonly image2D shadows;
layout(set = 3, binding = 0, rgba32f) uniform readonly image2D reflections;

float alphaDepth(uint idx)
{
    return alphaFragments.fragments[idx].depth;
}

vec4 alphaColor(uint idx)
{
    vec4 c = alphaFragments.fragments[idx].color;
    // return vec4(c.rgb * c.a, c.a);
    return c;
}

void swap(inout uint a, inout uint b)
{
    uint tmp = a;
    a = b;
    b = tmp;
}

void main()
{
    // Retrieve Opaque Color
    vec2 tCoords = gl_FragCoord.xy / textureSize(gBufferColors, 0);
    vec4 blendedColor = texture(gBufferColors, tCoords);

    // Retrieve all Alpha Fragments
    uint alphaHeadPtr = imageLoad(alphaHeadPointerTexture, ivec2(gl_FragCoord.xy)).r;
    const uint MAX_FRAGMENT_COUNT = 32;
    uint alphaFragmentIndices[MAX_FRAGMENT_COUNT];
    uint alphaFragmentIndexCount = 0;
    while (alphaHeadPtr > 0 && alphaFragmentIndexCount < MAX_FRAGMENT_COUNT) {
        alphaFragmentIndices[alphaFragmentIndexCount] = alphaHeadPtr;
        alphaHeadPtr = alphaFragments.fragments[alphaHeadPtr].next;
        ++alphaFragmentIndexCount;
    }

    // Sort Alpha Fragments by Depth (biggest depth first)
    if (alphaFragmentIndexCount > 1) {
        for (uint i = 0; i < alphaFragmentIndexCount - 1; i++) {
            for (uint j = 0; j < alphaFragmentIndexCount - i - 1; j++) {
                if (alphaDepth(alphaFragmentIndices[j]) > alphaDepth(alphaFragmentIndices[j + 1])) {
                    swap(alphaFragmentIndices[j], alphaFragmentIndices[j + 1]);
                }
            }
        }
    }

    // Blend with sorted fragments
    for (uint i = 0; i < alphaFragmentIndexCount; i++) {
        vec4 alphaFragColor = alphaColor(alphaFragmentIndices[i]);
        blendedColor = mix(blendedColor, alphaFragColor, alphaFragColor.a);
    }

    // Shadows
    float shadows = imageLoad(shadows, ivec2(gl_FragCoord.xy)).r;
    if (shadows > 0.0) { // We have shadows
        // Closest Hit Shader write 5.0 for Opaque shadows
        // Any Hit Shader writes between 0 and N for Alpha shadows
        // We therefore assume that >= 5 is fully dark
        float shadowFactor = min(shadows / 10.0, 1.0) * 0.9;
        blendedColor = mix(blendedColor, vec4(vec3(0.0), 1.0), shadowFactor);
    }

    // Reflections

    fragColor = blendedColor;
}

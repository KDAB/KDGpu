#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 vertexPosition;

struct RectFragmentData {
    vec2 pixelPosition;
    vec2 rectOffset;
    vec2 rectExtent;
    vec2 rectCorner;
    vec2 rectCenter;
};

layout(location = 0) out RectFragmentData rectFragmentData;

layout(set = 0, binding = 0) uniform GlobalData
{
    vec2 viewportSize;
}
globalData;

layout(set = 0, binding = 1) uniform RectData
{
    vec2 offset; // Pixel offset from top-left
    vec2 extent; // Pixel size
    float z;
}
rectData;

void main()
{
    vec2 pixelPosition = vertexPosition * rectData.extent + rectData.offset;
    vec2 ndcPosition = 2.0 * pixelPosition / globalData.viewportSize - vec2(1.0);

    gl_Position = vec4(ndcPosition, rectData.z, 1.0);

    // Pass along the pixel coordinates of the vertex being processed.
    rectFragmentData.pixelPosition = pixelPosition;
    rectFragmentData.rectOffset = rectData.offset;
    rectFragmentData.rectExtent = rectData.extent;
    rectFragmentData.rectCorner = rectData.extent / 2.0;
    rectFragmentData.rectCenter = rectFragmentData.rectOffset + rectFragmentData.rectCorner;
}

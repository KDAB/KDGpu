#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 vertexPosition;

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
    gl_Position.xy = 2.0 * pixelPosition / globalData.viewportSize - vec2(1.0);
    gl_Position.z = rectData.z;
    gl_Position.w = 1.0;
}

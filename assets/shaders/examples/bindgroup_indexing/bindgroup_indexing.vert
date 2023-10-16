#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;

layout(location = 0) out vec3 color;

// Array of Variable Length UBO
layout(set = 0, binding = 0) uniform Transform
{
    mat4 modelMatrix;
} transforms[];

// FrameCounter SSBO
layout(set = 1, binding = 0) coherent buffer FrameCounter
{
    uint primitiveProcessingCount;
} frameCounter;

layout(push_constant) uniform PushConstants {
    uint transformsCount;
} pushConstants;

void main()
{
    color = vertexColor;

    // Increase frameCounter for each invocation of the vertexShader
    frameCounter.primitiveProcessingCount += 1;

    // Since we have 3 vertices and each vertex shader increments
    // the actual frameCount is primitiveProcessingCount / 3
    uint frameIdx = (frameCounter.primitiveProcessingCount / 3);

    float angle =  mod(float(frameIdx), 360.0); // value between 0 and 359
    const float angleStep = 360.0 / float(pushConstants.transformsCount);

    // Select the right index based on current angle and steps between transforms
    // angle [0, 359] and angleStep (e.g 45)
    const uint transformIdx = uint(angle / angleStep);

    gl_Position = transforms[transformIdx].modelMatrix * vec4(vertexPosition, 1.0);
}

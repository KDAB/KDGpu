#version 450
#extension GL_ARB_separate_shader_objects : enable

struct RectFragmentData {
    vec2 pixelPosition;
    vec2 rectOffset;
    vec2 rectExtent;
    vec2 rectCorner;
    vec2 rectCenter;
};

layout(location = 0) in RectFragmentData rectFragmentData;

layout(location = 0) out vec4 fragColor;

const vec4 gray200 = vec4(0.898, 0.906, 0.922, 1.0);
const vec4 emerald500 = vec4(0.063, 0.725, 0.506, 1.0);
const vec4 blue500 = vec4(0.231, 0.51, 0.965, 1.0);
const vec4 violet500 = vec4(0.545, 0.361, 0.965, 1.0);
const vec4 pink500 = vec4(0.925, 0.282, 0.6, 1.0);

const float borderTop = 2.0;
const float borderRight = 4.0;
const float borderBottom = 2.0;
const float borderLeft = 12.0;

void main()
{
    vec2 borderCorner = rectFragmentData.rectCorner;
    if (rectFragmentData.pixelPosition.y >= rectFragmentData.rectCenter.y) {
        borderCorner.y -= borderBottom;
    } else {   
        borderCorner.y -= borderTop;
    }
   
    if (rectFragmentData.pixelPosition.x >= rectFragmentData.rectCenter.x) {
        borderCorner.x -= borderRight;
    } else {
        borderCorner.x -= borderLeft;
    }
    
    
    if (rectFragmentData.pixelPosition.x > rectFragmentData.rectCenter.x + borderCorner.x)
        fragColor = pink500;
    else if (rectFragmentData.pixelPosition.x < rectFragmentData.rectCenter.x - borderCorner.x)
        fragColor = blue500;
    else if (rectFragmentData.pixelPosition.y > rectFragmentData.rectCenter.y + borderCorner.y)
        fragColor = violet500;
    else if (rectFragmentData.pixelPosition.y < rectFragmentData.rectCenter.y - borderCorner.y)
        fragColor = emerald500;
    else 
        fragColor = gray200;
    
    // Hard-wired gray-200 color for now
    // fragColor = gray200;
}

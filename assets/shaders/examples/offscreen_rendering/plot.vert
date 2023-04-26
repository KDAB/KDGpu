#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 vertexPos;

out gl_PerVertex
{
    vec4 gl_Position;
    float gl_PointSize;
};

void main()
{
    gl_PointSize = 16.0;
    gl_Position = vertexPos;
}

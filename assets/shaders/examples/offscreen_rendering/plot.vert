#version 450

layout(location = 0) in vec4 vertexPos;

out gl_PerVertex
{
    vec4 gl_Position;
    float gl_PointSize;
};

void main()
{
    gl_PointSize = 2.0;
    gl_Position = vertexPos;
}

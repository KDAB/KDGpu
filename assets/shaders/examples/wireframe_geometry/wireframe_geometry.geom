#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(location = 0) in vec3 v_position[];
layout(location = 1) in vec3 v_normal[];

layout(location = 0) out vec3 g_position;
layout(location = 1) out vec3 g_normal;
layout(location = 2) noperspective out vec3 g_edgeDistance;

layout(set = 0, binding = 1) uniform Viewport
{
    mat4 viewportMatrix;
}
vp;

void main()
{
    // Transform each vertex into viewport space
    vec2 p0 = vec2(vp.viewportMatrix * (gl_in[0].gl_Position / gl_in[0].gl_Position.w));
    vec2 p1 = vec2(vp.viewportMatrix * (gl_in[1].gl_Position / gl_in[1].gl_Position.w));
    vec2 p2 = vec2(vp.viewportMatrix * (gl_in[2].gl_Position / gl_in[2].gl_Position.w));

    // Calculate lengths of 3 edges of triangle
    float a = length(p1 - p2);
    float b = length(p2 - p0);
    float c = length(p1 - p0);

    // Calculate internal angles using the cosine rule
    float alpha = acos((b * b + c * c - a * a) / (2.0 * b * c));
    float beta = acos((a * a + c * c - b * b) / (2.0 * a * c));

    // Calculate the perpendicular distance of each vertex from the opposing edge
    float ha = abs(c * sin(beta));
    float hb = abs(c * sin(alpha));
    float hc = abs(b * sin(alpha));

    // Now add this perpendicular distance as a per-vertex property in addition to
    // the position and normal calculated in the vertex shader.

    // Vertex 0 (a)
    g_edgeDistance = vec3(ha, 0, 0);
    g_normal = v_normal[0];
    g_position = v_position[0];
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    // Vertex 1 (b)
    g_edgeDistance = vec3(0, hb, 0);
    g_normal = v_normal[1];
    g_position = v_position[1];
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();

    // Vertex 2 (c)
    g_edgeDistance = vec3(0, 0, hc);
    g_normal = v_normal[2];
    g_position = v_position[2];
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();

    // Finish the primitive off
    EndPrimitive();
}

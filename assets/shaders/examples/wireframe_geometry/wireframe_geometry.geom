#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(location = 0) in vec3 v_position[];
layout(location = 1) in vec3 v_normal[];
layout(location = 2) in float v_excludeEdge[];

layout(location = 0) out vec3 g_position;
layout(location = 1) out vec3 g_normal;
layout(location = 2) noperspective out vec4 g_edgeA;
layout(location = 3) noperspective out vec4 g_edgeB;
layout(location = 4) flat out vec3 g_excludeEdge;
layout(location = 5) flat out int g_edgeCase;

layout(set = 0, binding = 1) uniform Viewport
{
    mat4 viewportMatrix;
}
vp;

const int infoA[] = int[](0, 0, 0, 0, 1, 1, 2);
const int infoB[] = int[](1, 1, 2, 0, 2, 1, 2);
const int infoAd[] = int[](2, 2, 1, 1, 0, 0, 0);
const int infoBd[] = int[](2, 2, 1, 2, 0, 2, 1);

// For excluded edges, we set a very high edge distance so that
// the fragment shader will never draw them.
const float maxEdgeWidth = 100.0;

vec2 transformToViewport(const in vec4 p)
{
    return vec2(vp.viewportMatrix * (p / p.w));
}

void main()
{
    g_edgeCase = int(gl_in[0].gl_Position.z < 0) * int(4) + int(gl_in[1].gl_Position.z < 0) * int(2) + int(gl_in[2].gl_Position.z < 0);

    // If all vertices are behind us, cull the primitive
    if (g_edgeCase == 7)
        return;

    // Transform each vertex into viewport space
    vec2 p[3];
    p[0] = transformToViewport(gl_in[0].gl_Position);
    p[1] = transformToViewport(gl_in[1].gl_Position);
    p[2] = transformToViewport(gl_in[2].gl_Position);

    if (g_edgeCase == 0) {
        // Common case where all vertices are within the viewport
        g_edgeA = vec4(0.0);
        g_edgeB = vec4(0.0);

        // Calculate lengths of 3 edges of triangle
        float a = length(p[1] - p[2]);
        float b = length(p[2] - p[0]);
        float c = length(p[1] - p[0]);

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
        g_edgeA = vec4(ha, v_excludeEdge[1] * maxEdgeWidth, v_excludeEdge[2] * maxEdgeWidth, 0.0);
        g_normal = v_normal[0];
        g_position = v_position[0];
        gl_Position = gl_in[0].gl_Position;
        EmitVertex();

        // Vertex 1 (b)
        g_edgeA = vec4(v_excludeEdge[0] * maxEdgeWidth, hb, v_excludeEdge[2] * maxEdgeWidth, 0.0);
        g_normal = v_normal[1];
        g_position = v_position[1];
        gl_Position = gl_in[1].gl_Position;
        EmitVertex();

        // Vertex 2 (c)
        g_edgeA = vec4(v_excludeEdge[0] * maxEdgeWidth, v_excludeEdge[1] * maxEdgeWidth, hc, 0.0);
        g_normal = v_normal[2];
        g_position = v_position[2];
        gl_Position = gl_in[2].gl_Position;
        EmitVertex();

        // Finish the primitive off
        EndPrimitive();
    } else {
        // Viewport projection breaks down for one or two vertices.
        // Calculate what we can here and defer rest to fragment shader.
        // Since this is coherent for the entire primitive the conditional
        // in the fragment shader is still cheap as all concurrent
        // fragment shader invocations will take the same code path.

        // Pass through which edges are excluded from the wireframe. We will use
        // this in the fragment shader to avoid drawing excluded edges.
        g_excludeEdge = vec3(v_excludeEdge[0], v_excludeEdge[1], v_excludeEdge[2]);

        // Copy across the viewport-space points for the (up to) two vertices
        // in the viewport
        g_edgeA.xy = p[infoA[g_edgeCase]];
        g_edgeB.xy = p[infoB[g_edgeCase]];

        // Copy across the viewport-space edge vectors for the (up to) two vertices
        // in the viewport
        g_edgeA.zw = normalize(g_edgeA.xy - p[infoAd[g_edgeCase]]);
        g_edgeB.zw = normalize(g_edgeB.xy - p[infoBd[g_edgeCase]]);

        // Pass through the other vertex attributes
        g_normal = v_normal[0];
        g_position = v_position[0];
        gl_Position = gl_in[0].gl_Position;
        EmitVertex();

        g_normal = v_normal[1];
        g_position = v_position[1];
        gl_Position = gl_in[1].gl_Position;
        EmitVertex();

        g_normal = v_normal[2];
        g_position = v_position[2];
        gl_Position = gl_in[2].gl_Position;
        EmitVertex();

        // Finish the primitive off
        EndPrimitive();
    }
}

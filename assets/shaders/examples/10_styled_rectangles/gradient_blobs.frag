#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) uniform GradientStops
{
    vec4 colors[4];
    vec2 positions[4]; // Padded out to vec4 by std140 rules
} gradientStops;

void main()
{
    vec2 Q = gradientStops.positions[0] - gradientStops.positions[2];
    vec2 R = gradientStops.positions[1] - gradientStops.positions[0];
    vec2 S = R + gradientStops.positions[2] - gradientStops.positions[3];
    vec2 T = gradientStops.positions[0] - texCoord;
    float u;
    float t;

    if (Q.x == 0.0 && S.x == 0.0) {
        u = -T.x/R.x;
        t = (T.y + u * R.y) / (Q.y + u * S.y);
    } else if (Q.y == 0.0 && S.y == 0.0) {
        u = -T.y / R.y;
        t = (T.x + u * R.x) / (Q.x + u * S.x);
    } else {
        float A = S.x * R.y - R.x * S.y;
        float B = S.x * T.y - T.x * S.y + Q.x * R.y - R.x * Q.y;
        float C = Q.x * T.y - T.x * Q.y;
        // Solve Au^2 + Bu + C = 0
        if (abs(A) < 0.0001)
            u = -C / B;
        else
            u = (-B + sqrt(B * B - 4.0 * A * C)) / (2.0 * A);
        t = (T.y + u * R.y) / (Q.y + u * S.y);
    }
    u = clamp(u, 0.0, 1.0);
    t = clamp(t, 0.0, 1.0);

    // These two lines smooth out t and u to avoid visual 'lines' at the boundaries.
    // They can be removed to improve performance at the cost of graphical quality.
    t = smoothstep(0.0, 1.0, t);
    u = smoothstep(0.0, 1.0, u);

    vec4 colorA = mix(gradientStops.colors[0], gradientStops.colors[1], u);
    vec4 colorB = mix(gradientStops.colors[2], gradientStops.colors[3], u);
    fragColor = mix(colorA, colorB, t);
}

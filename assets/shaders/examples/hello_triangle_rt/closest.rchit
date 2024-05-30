#version 460 core
#extension GL_EXT_ray_tracing : enable

layout(location = 0) rayPayloadInEXT vec4 payload;
hitAttributeEXT vec2 rayAttributes;

void main()
{
    const vec3 barycentricCoords = vec3(1.0f - rayAttributes.x - rayAttributes.y, rayAttributes.x, rayAttributes.y);
    payload = vec4(barycentricCoords, 1.0);
}

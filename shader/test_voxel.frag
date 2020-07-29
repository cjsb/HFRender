#version 450 core
out vec4 FragColor;

in vec3 worldPosition;

uniform vec4 albedo;
layout(rgba8) uniform image3D texture3D;

vec3 scaleAndBias(vec3 p) { return 0.5f * p + vec3(0.5f); }

bool isInsideCube(const vec3 p, float e) { return abs(p.x) < 1 + e && abs(p.y) < 1 + e && abs(p.z) < 1 + e; }

void main()
{
    FragColor = albedo;

    //if(!isInsideCube(worldPosition, 0)) return;

    ivec3 dim = imageSize(texture3D);
    vec3 voxel = scaleAndBias(worldPosition);
    imageStore(texture3D, ivec3(dim * voxel), albedo);
}
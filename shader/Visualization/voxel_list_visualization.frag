#version 450 core
out vec4 fragColor;

layout(rgba8) uniform imageBuffer u_voxelListColor;
//layout(rgba8) uniform imageBuffer u_voxelListNormal;

flat in int frag_idx;

void main()
{
	fragColor = imageLoad(u_voxelListColor, frag_idx);
}
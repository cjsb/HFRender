#version 450 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

uniform int u_numNode;
uniform int u_start;
uniform int u_level;
uniform int u_octreeLevel;

uniform vec4 u_emptyColor;

layout(r32ui) uniform uimageBuffer u_octreeNodeIdx;
layout(rgb10_a2ui) uniform uimageBuffer u_octreeNodeBrickIdx;
layout(r32ui) uniform uimage3D u_octreeBrickValue;

#include "utils.glsl"
#include "mipmap_utils.glsl"

void main()
{
	uint thxId = gl_GlobalInvocationID.y * 1024 + gl_GlobalInvocationID.x;
	if (thxId >= u_numNode)
		return;

	uint nodeIdx = imageLoad(u_octreeNodeIdx, u_start + int(thxId)).r;
	uint chlidIdx = nodeIdx & NODE_MASK_INDEX;
	if (chlidIdx == 0)
	{
		return;
	}

	loadChildTile(int(chlidIdx));

	uvec4 brick_idx = imageLoad(u_octreeNodeBrickIdx, u_start + int(thxId));
	ivec3 brickAddress = ivec3(brick_idx.xyz);

	for (int x = 0; x <= 4; x++)
	{
		for (int y = 0; y <= 4; y++)
		{
			for (int z = 0; z <= 4; z++)
			{
				ivec3 coord = ivec3(x, y, z);
				vec4 color = mipmapIsotropic(coord);
				uint color_v = convVec4ToRGBA8(color);

				ivec3 loc = coord / 2;
				imageStore(u_octreeBrickValue, brickAddress + loc, uvec4(color_v, 0, 0, 0));
			}
		}
	}
}
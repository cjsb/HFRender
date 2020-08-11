#version 450 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

uniform int u_numNode;
uniform int u_start;
uniform int u_level;
uniform int u_octreeLevel;

uniform vec4 u_emptyColor = vec4(0);

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

	loadChildTile(chlidIdx);

	vec4 color = mipmapIsotropic(ivec3(2, 2, 2));
	uint color_v = convVec4ToRGBA8(color);

	uvec4 brick_idx = imageLoad(u_octreeNodeBrickIdx, u_start + int(thxId));
	imageStore(u_octreeBrickValue, brick_idx + ivec3(1, 1, 1), uvec4(color_v, 0, 0, 0));
}
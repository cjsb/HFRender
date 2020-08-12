#version 450 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(r32ui) uniform uimageBuffer u_octreeNodeIdx;
layout(rgb10_a2ui) uniform uimageBuffer u_octreeNodeBrickIdx;

layout(r32ui) uniform uimage3D u_octreeBrickColor;
layout(r32ui) uniform uimage3D u_octreeBrickIrradiance;

uniform sampler2D u_shadowMap;

uniform int u_voxelDim;
uniform int u_octreeLevel;

struct DirectionLight {
	vec3 direction;
	vec3 color;
};

uniform DirectionLight u_light;

#include "utils.glsl"

void main()
{
	ivec2 mapSize = textureSize(u_lightMap, 0);
	if (gl_GlobalInvocationID.x >= mapSize.x || gl_GlobalInvocationID.y >= mapSize.y)
		return;
	
	vec2 uv = vec2(gl_GlobalInvocationID.x / float(mapSize.x), gl_GlobalInvocationID.y / float(mapSize.y));
	vec3 pos = texture(u_shadowMap, uv).xyz; //-1~1
	if (pos.x == 0 && pos.y == 0 && pos.z == 0)
	{
		return;
	}

	pos = pos * 0.5 + 0.5; //0~1
	if (pos.x < 0 || pos.y < 0 || pos.z < 0 ||
		pos.x > 1 || pos.y > 1 || pos.z > 1)
	{
		return;
	}

	uvec3 loc = uvec3(pos * u_voxelDim);
	int childIdx = 0;
	uint node;
	ivec3 offset;
	uint voxelDim = u_voxelDim;
	uvec3 umin = uvec3(0, 0, 0);

	node = imageLoad(u_octreeNodeIdx, childIdx).r;
	for (int i = 0; i < u_octreeLevel; ++i)
	{
		voxelDim /= 2;
		int nextChildIdx = int(node & NODE_MASK_INDEX);  //mask out flag bit to get child idx
		if (nextChildIdx == 0)
		{
			break;
		}
		childIdx = nextChildIdx;

		offset.x = clamp(int(1 + loc.x - umin.x - voxelDim), 0, 1);
		offset.y = clamp(int(1 + loc.y - umin.y - voxelDim), 0, 1);
		offset.z = clamp(int(1 + loc.z - umin.z - voxelDim), 0, 1);

		childIdx += offset.x + 2 * offset.y + 4 * offset.z;
		umin += voxelDim * offset;

		node = imageLoad(u_octreeNodeIdx, childIdx).r;
	}

	if ((node & NODE_MASK_CHILD) == 0)
	{
		//正确的话是不会执行到这里的
		return;
	}

	//获取最后一层voxel（octreeLevel）在叶子节点中的offset
	offset.x = clamp(int(1 + loc.x - umin.x - voxelDim), 0, 1);
	offset.y = clamp(int(1 + loc.y - umin.y - voxelDim), 0, 1);
	offset.z = clamp(int(1 + loc.z - umin.z - voxelDim), 0, 1);

	uvec4 brick_idx = imageLoad(u_octreeNodeBrickIdx, childIdx);
	ivec3 brick_coord = ivec3(brick_idx.xyz) + 2 * offset;

	uint voxelColor_v = imageLoad(u_octreeBrickColor, brick_coord).r;
	vec4 voxelColor = convRGBA8ToVec4(voxelColor_v);
	voxelColor.rgb /= 255.0;

	vec4 reflectedRadiance = vec4(u_light.color * voxelColor.rgb, 1);
	reflectedRadiance.rgb *= 255.0;
	uint reflectedRadiance_v = convVec4ToRGBA8(reflectedRadiance);
	
	imageStore(u_octreeBrickIrradiance, brick_coord, uvec4(reflectedRadiance_v, 0, 0, 0));
}
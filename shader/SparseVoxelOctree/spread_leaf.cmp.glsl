#version 450 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

uniform int u_numLeafNode;
uniform int u_leafStart;

layout(rgb10_a2ui) uniform uimageBuffer u_octreeNodeBrickIdx;
layout(r32ui) uniform uimage3D u_octreeBrickValue;

#include "utils.glsl"

vec4 voxelValues[8] = {
  vec4(0),
  vec4(0),
  vec4(0),
  vec4(0),
  vec4(0),
  vec4(0),
  vec4(0),
  vec4(0)
};

void loadVoxelValues(ivec3 brickAddress) {
	// Collect the original voxel colors (from voxelfragmentlist-voxels)
	// which were stored at the corners of the brick texture.
	for (int i = 0; i < 8; ++i) {
		uint val = imageLoad(u_octreeBrickValue, brickAddress + 2 * ivec3(childOffsets[i])).r;
		voxelValues[i] = convRGBA8ToVec4(val);
		voxelValues[i].w = 0;
	}
}

void main()
{
	uint thxId = gl_GlobalInvocationID.y * 1024 + gl_GlobalInvocationID.x;
	if (thxId >= u_numLeafNode)
		return;

	uvec4 brick_idx = imageLoad(u_octreeNodeBrickIdx, u_leafStart + int(thxId));
	ivec3 brickAddress = ivec3(brick_idx.xyz);

	loadVoxelValues(brickAddress);
	
	vec4 col = vec4(0);
	uint col_v;

	// Center
	for (int i = 0; i < 8; ++i) {
		col += 0.125 * voxelValues[i];
	}

	col_v = convVec4ToRGBA8(col);
	imageStore(u_octreeBrickValue, brickAddress + ivec3(1, 1, 1), uvec4(col_v, 0, 0, 0));


	// Face X
	col = vec4(0);
	col += 0.25 * voxelValues[1];
	col += 0.25 * voxelValues[3];
	col += 0.25 * voxelValues[5];
	col += 0.25 * voxelValues[7];
	col_v = convVec4ToRGBA8(col);
	imageStore(u_octreeBrickValue, brickAddress + ivec3(2, 1, 1), uvec4(col_v, 0, 0, 0));

	// Face X Neg
	col = vec4(0);
	col += 0.25 * voxelValues[0];
	col += 0.25 * voxelValues[2];
	col += 0.25 * voxelValues[4];
	col += 0.25 * voxelValues[6];
	col_v = convVec4ToRGBA8(col);
	imageStore(u_octreeBrickValue, brickAddress + ivec3(0, 1, 1), uvec4(col_v, 0, 0, 0));


	// Face Y
	col = vec4(0);
	col += 0.25 * voxelValues[2];
	col += 0.25 * voxelValues[3];
	col += 0.25 * voxelValues[6];
	col += 0.25 * voxelValues[7];
	col_v = convVec4ToRGBA8(col);
	imageStore(u_octreeBrickValue, brickAddress + ivec3(1, 2, 1), uvec4(col_v, 0, 0, 0));

	// Face Y Neg
	col = vec4(0);
	col += 0.25 * voxelValues[0];
	col += 0.25 * voxelValues[1];
	col += 0.25 * voxelValues[4];
	col += 0.25 * voxelValues[5];
	col_v = convVec4ToRGBA8(col);
	imageStore(u_octreeBrickValue, brickAddress + ivec3(1, 0, 1), uvec4(col_v, 0, 0, 0));


	// Face Z
	col = vec4(0);
	col += 0.25 * voxelValues[4];
	col += 0.25 * voxelValues[5];
	col += 0.25 * voxelValues[6];
	col += 0.25 * voxelValues[7];
	col_v = convVec4ToRGBA8(col);
	imageStore(u_octreeBrickValue, brickAddress + ivec3(1, 1, 2), uvec4(col_v, 0, 0, 0));

	// Face Z Neg
	col = vec4(0);
	col += 0.25 * voxelValues[0];
	col += 0.25 * voxelValues[1];
	col += 0.25 * voxelValues[2];
	col += 0.25 * voxelValues[3];
	col_v = convVec4ToRGBA8(col);
	imageStore(u_octreeBrickValue, brickAddress + ivec3(1, 1, 0), uvec4(col_v, 0, 0, 0));


	// Edges
	col = vec4(0);
	col += 0.5 * voxelValues[0];
	col += 0.5 * voxelValues[1];
	col_v = convVec4ToRGBA8(col);
	imageStore(u_octreeBrickValue, brickAddress + ivec3(1, 0, 0), uvec4(col_v, 0, 0, 0));

	col = vec4(0);
	col += 0.5 * voxelValues[0];
	col += 0.5 * voxelValues[2];
	col_v = convVec4ToRGBA8(col);
	imageStore(u_octreeBrickValue, brickAddress + ivec3(0, 1, 0), uvec4(col_v, 0, 0, 0));

	col = vec4(0);
	col += 0.5 * voxelValues[2];
	col += 0.5 * voxelValues[3];
	col_v = convVec4ToRGBA8(col);
	imageStore(u_octreeBrickValue, brickAddress + ivec3(1, 2, 0), uvec4(col_v, 0, 0, 0));

	col = vec4(0);
	col += 0.5 * voxelValues[3];
	col += 0.5 * voxelValues[1];
	col_v = convVec4ToRGBA8(col);
	imageStore(u_octreeBrickValue, brickAddress + ivec3(2, 1, 0), uvec4(col_v, 0, 0, 0));

	col = vec4(0);
	col += 0.5 * voxelValues[0];
	col += 0.5 * voxelValues[4];
	col_v = convVec4ToRGBA8(col);
	imageStore(u_octreeBrickValue, brickAddress + ivec3(0, 0, 1), uvec4(col_v, 0, 0, 0));

	col = vec4(0);
	col += 0.5 * voxelValues[2];
	col += 0.5 * voxelValues[6];
	col_v = convVec4ToRGBA8(col);
	imageStore(u_octreeBrickValue, brickAddress + ivec3(0, 2, 1), uvec4(col_v, 0, 0, 0));

	col = vec4(0);
	col += 0.5 * voxelValues[3];
	col += 0.5 * voxelValues[7];
	col_v = convVec4ToRGBA8(col);
	imageStore(u_octreeBrickValue, brickAddress + ivec3(2, 2, 1), uvec4(col_v, 0, 0, 0));

	col = vec4(0);
	col += 0.5 * voxelValues[1];
	col += 0.5 * voxelValues[5];
	col_v = convVec4ToRGBA8(col);
	imageStore(u_octreeBrickValue, brickAddress + ivec3(2, 0, 1), uvec4(col_v, 0, 0, 0));

	col = vec4(0);
	col += 0.5 * voxelValues[4];
	col += 0.5 * voxelValues[6];
	col_v = convVec4ToRGBA8(col);
	imageStore(u_octreeBrickValue, brickAddress + ivec3(0, 1, 2), uvec4(col_v, 0, 0, 0));

	col = vec4(0);
	col += 0.5 * voxelValues[6];
	col += 0.5 * voxelValues[7];
	col_v = convVec4ToRGBA8(col);
	imageStore(u_octreeBrickValue, brickAddress + ivec3(1, 2, 2), uvec4(col_v, 0, 0, 0));

	col = vec4(0);
	col += 0.5 * voxelValues[5];
	col += 0.5 * voxelValues[7];
	col_v = convVec4ToRGBA8(col);
	imageStore(u_octreeBrickValue, brickAddress + ivec3(2, 1, 2), uvec4(col_v, 0, 0, 0));

	col = vec4(0);
	col += 0.5 * voxelValues[4];
	col += 0.5 * voxelValues[5];
	col_v = convVec4ToRGBA8(col);
	imageStore(u_octreeBrickValue, brickAddress + ivec3(1, 0, 2), uvec4(col_v, 0, 0, 0));
}
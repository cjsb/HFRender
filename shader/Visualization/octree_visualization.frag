#version 450 core
out vec4 color;

in vec3 worldPosition;

layout(r32ui) uniform uimageBuffer u_octreeIdx;
uniform int u_voxelDim;
uniform int u_level;

void main()
{
	uvec3 umin, umax;
	uvec4 loc;
	int childIdx = 0;
	uint node, subnode;
	uint voxelDim = u_voxelDim;
	bool bFlag = true;

	//Get the voxel coordinate of voxel loaded by this thread
	loc = uvec4(worldPosition * u_voxelDim, 0);

	umin = uvec3(0, 0, 0);
	umax = uvec3(voxelDim, voxelDim, voxelDim);

	node = imageLoad(u_octreeIdx, childIdx).r;

	for (int i = 0; i < u_level; ++i)
	{
		voxelDim /= 2;
		if ((node & 0x80000000) == 0)
		{
			bFlag = false;
			break;
		}
		childIdx = int(node & 0x7FFFFFFF);  //mask out flag bit to get child idx

		subnode = clamp(int(1 + loc.x - umin.x - voxelDim), 0, 1);
		subnode += 4 * clamp(int(1 + loc.y - umin.y - voxelDim), 0, 1);
		subnode += 2 * clamp(int(1 + loc.z - umin.z - voxelDim), 0, 1);
		childIdx += int(subnode);

		umin.x += voxelDim * clamp(int(1 + loc.x - umin.x - voxelDim), 0, 1);
		umin.y += voxelDim * clamp(int(1 + loc.y - umin.y - voxelDim), 0, 1);
		umin.z += voxelDim * clamp(int(1 + loc.z - umin.z - voxelDim), 0, 1);

		node = imageLoad(u_octreeIdx, childIdx).r;
	}
	if (bFlag && (node & 0x80000000) != 0)
	{
		color = vec4(0,1,1,1);
	}
	else
	{
		discard;
	}
}
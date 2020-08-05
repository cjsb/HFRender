#version 450 core
layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

uniform int u_start;
uniform int u_allocStart;
uniform int u_num;

layout(r32ui) uniform uimageBuffer u_octreeIdx;

//atomic counter 
layout(binding = 0, offset = 0) uniform atomic_uint u_allocCount;

void main()
{
	uint offset;
	uint thxId = gl_GlobalInvocationID.x;
	if (thxId >= u_num)
		return;

	//get child pointer
	uint childIdx = imageLoad(u_octreeIdx, u_start + int(thxId)).r;

	if ((childIdx & 0x80000000) != 0) //need to allocate
	{
		offset = atomicCounterIncrement(u_allocCount);
		offset *= 8; //one tile has eight nodes
		offset += u_allocStart; //Add allocation offset 
		offset |= 0x80000000;    //Set the most significant bit
		imageStore(u_octreeIdx, u_start + int(thxId), uvec4(offset, 0, 0, 0));
	}
}
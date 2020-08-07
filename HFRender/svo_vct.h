#pragma once
#include <iostream>
#include "opengl_common.h"
#include "world.h"
#include "automic_buffer.h"

class SVO_VCT
{
public:
	SVO_VCT();
	~SVO_VCT();

	void SparseVoxelize(World& world);

private:
	void BuildVoxelList(World& world);
	void FlagNode();
	void AllocateNode();
	void FindNeighbours();
	void AllocateBrick();
	void WriteLeafNode();
	void SpreadLeafBrick();
	void BorderTransfer();

	uint32_t m_voxel_size;
	uint32_t m_numVoxelFrag;

	FramebufferPtr m_voxel_FBO;

	AutomicBufferPtr m_voxel_list_count;
	TextureBufferPtr m_voxel_list_pos;
	TextureBufferPtr m_voxel_list_color;
	TextureBufferPtr m_voxel_list_normal;

};
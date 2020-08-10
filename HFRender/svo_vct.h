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
	void BuildSVO();
	void AllocBrick();
	void WriteLeafNode();
	void AllocateNode();
	void FindNeighbours();
	void AllocateBrick();
	void WriteLeafNode();
	void SpreadLeafBrick();
	void BorderTransfer();

	uint32_t m_voxelSize;
	uint32_t m_octreeLevel;
	uint32_t m_brickPoolDim;

	uint32_t m_numVoxelFrag;
	uint32_t m_numOctreeNoLeafNode;

	FramebufferPtr m_voxel_FBO;

	AutomicBufferPtr m_voxel_list_count;
	TextureBufferPtr m_voxel_list_pos;
	TextureBufferPtr m_voxel_list_color;
	TextureBufferPtr m_voxel_list_normal;

	AutomicBufferPtr m_octree_node_count;
	TextureBufferPtr m_octree_node_idx;
	TextureBufferPtr m_octree_node_brick_idx;

	Texture3DPtr m_octree_brick_color;
	Texture3DPtr m_octree_brick_normal;
	Texture3DPtr m_octree_brick_irradiance;
};
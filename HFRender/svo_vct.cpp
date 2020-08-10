#include "svo_vct.h"
#include "config.h"

SVO_VCT::SVO_VCT()
{
	m_voxelSize = Config::Instance()->voxelSize;
	m_octreeLevel = log2f(m_voxelSize);
	m_brickPoolDim = Config::Instance()->brickPoolDim;

	m_voxel_list_count = std::make_shared<AutomicBuffer>(0);
	m_octree_node_count = std::make_shared<AutomicBuffer>(0);

	m_voxel_FBO = std::make_shared<Framebuffer>();
	m_voxel_FBO->AttachColorBuffer(std::make_unique<RenderSurface>(m_voxelSize, m_voxelSize, GL_RGB));
}

SVO_VCT::~SVO_VCT()
{

}

void SVO_VCT::SparseVoxelize(World& world)
{
	BuildVoxelList(world);


}

void SVO_VCT::BuildVoxelList(World& world)
{
	m_voxel_list_count->BindBase(0);

	ViewContext voxel_vc;
	voxel_vc.SetColorMask(glm::bvec4(false));
	voxel_vc.SetDepthStates(false, false, GL_LESS);
	voxel_vc.SetCullFace(false, GL_BACK);
	voxel_vc.SetBlend(false);
	voxel_vc.SetFramebuffer(m_voxel_FBO);

	glViewport(0, 0, m_voxelSize, m_voxelSize);
	world.CommitRenderContext(voxel_vc);

	// Obtain number of voxel fragments
	voxel_vc.FlushRenderContext(false);
	m_voxel_list_count->Sync();
	m_numVoxelFrag = m_voxel_list_count->GetVal();
	m_voxel_list_count->SetVal(0);
	std::cout << "Number of Entries in Voxel Fragment List: " << m_numVoxelFrag << std::endl;

	// Create buffers for voxel fragment list
	m_voxel_list_pos = std::make_shared<TextureBuffer>(nullptr, sizeof(GLuint) * m_numVoxelFrag, GL_R32UI);
	m_voxel_list_pos->SetUnit(0);
	m_voxel_list_pos->SetInternalFormat(GL_RGB10_A2UI);
	m_voxel_list_color = std::make_shared<TextureBuffer>(nullptr, sizeof(GLuint) * m_numVoxelFrag, GL_RGBA8);
	m_voxel_list_color->SetUnit(1);
	m_voxel_list_normal = std::make_shared<TextureBuffer>(nullptr, sizeof(GLuint) * m_numVoxelFrag, GL_RGBA8);
	m_voxel_list_normal->SetUnit(2);

	ParamTable param = {
		{"u_bStore", 1}
	};
	TextureParamTable image_param = {
		{"u_voxelListPos", m_voxel_list_pos},
		{"u_voxelListColor", m_voxel_list_color},
		{"u_voxelListNormal", m_voxel_list_normal},
	};
	world.UpdateMaterialParam(param, {}, image_param);

	//Voxelize the scene again, this time store the data in the voxel fragment list
	voxel_vc.FlushRenderContext(false);
	m_voxel_list_pos->SyncImage();
}

void SVO_VCT::BuildSVO()
{
	std::vector<unsigned int> allocList; //the vector records the number of nodes in each tree level
	allocList.push_back(1); //root level has one node

	//Calculate the maximum possilbe node number
	int totalNode = 1;
	int nTmp = 1;
	for (int i = 1; i <= m_octreeLevel; ++i)
	{
		nTmp *= 8;
		totalNode += nTmp;
	}

	std::vector<GLuint> init_data(totalNode, 0);

	m_octree_node_idx = std::make_shared<TextureBuffer>(init_data.data(), sizeof(GLuint) * totalNode, GL_R32UI);
	m_octree_node_idx->SetUnit(0);

	m_octree_node_brick_idx = std::make_shared<TextureBuffer>(init_data.data(), sizeof(GLuint) * totalNode, GL_R32UI);
	m_octree_node_brick_idx->SetUnit(1);
	m_octree_node_brick_idx->SetInternalFormat(GL_RGB10_A2UI);

	m_voxel_list_pos->SetUnit(2);
	m_voxel_list_pos->SetImageAccess(GL_READ_ONLY);
	m_voxel_list_pos->SetUnit(3);
	m_voxel_list_color->SetImageAccess(GL_READ_ONLY);
	m_voxel_list_pos->SetUnit(4);
	m_voxel_list_normal->SetImageAccess(GL_READ_ONLY);
	m_octree_node_count->BindBase(0);

	ShaderPtr nodeFlagShader = ShaderLoader::Instance()->LoadShader(Config::Instance()->project_path + "shader/node_flag.cmp.glsl");
	ShaderPtr nodeAllocShader = ShaderLoader::Instance()->LoadShader(Config::Instance()->project_path + "shader/node_alloc.cmp.glsl");
	
	int nodeOffset = 0;//nodeOffset is the starting node in the current level
	int allocOffset = 1;//allocOffset is the starting address of remaining free space

	int dataWidth = 1024;
	int dataHeight = (m_numVoxelFrag + 1023) / dataWidth;
	int groupDimX = dataWidth / 8;
	int groupDimY = (dataHeight + 7) / 8;
	//叶子节点不分配nodeTile
	for (int i = 0; i < m_octreeLevel; ++i)
	{
		//node flag
		nodeFlagShader->Use();
		nodeFlagShader->SetInteger("u_numVoxelFrag", m_numVoxelFrag);
		nodeFlagShader->SetInteger("u_level", i);
		nodeFlagShader->SetInteger("u_voxelDim", m_voxelSize);
		nodeFlagShader->SetImage("u_voxelListPos", m_voxel_list_pos);
		nodeFlagShader->SetImage("u_octreeNodeIdx", m_octree_node_idx);
		glDispatchCompute(groupDimX, groupDimY, 1);
		m_octree_node_idx->SyncImage();

		//node tile allocation
		int numThread = allocList[i];
		nodeAllocShader->Use();
		nodeAllocShader->SetInteger("u_num", numThread);
		nodeAllocShader->SetInteger("u_start", nodeOffset);
		nodeAllocShader->SetInteger("u_allocStart", allocOffset);
		nodeAllocShader->SetImage("u_octreeNodeIdx", m_octree_node_idx);
		int allocGroupDim = (allocList[i] + 63) / 64;
		glDispatchCompute(allocGroupDim, 1, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);

		//Get the number of node tiles to allocate in the next level
		GLuint tileAllocated = m_octree_node_count->GetVal();
		m_octree_node_count->SetVal(0);

		//update offsets for next level
		int nodeAllocated = tileAllocated * 8;
		allocList.push_back(nodeAllocated); //titleAllocated * 8 is the number of threads we want to launch in the next level
		nodeOffset += allocList[i]; 
		allocOffset += nodeAllocated; 
	}

	m_numOctreeNoLeafNode = nodeOffset;
}

void SVO_VCT::AllocBrick()
{
	m_octree_brick_color = std::make_shared<Texture3D>(m_brickPoolDim, m_brickPoolDim, m_brickPoolDim, GL_RGBA8, GL_RGBA);
	m_octree_brick_color->SetUnit(0);

	m_octree_brick_normal = std::make_shared<Texture3D>(m_brickPoolDim, m_brickPoolDim, m_brickPoolDim, GL_RGBA8, GL_RGBA);
	m_octree_brick_normal->SetUnit(1);

	m_octree_brick_irradiance = std::make_shared<Texture3D>(m_brickPoolDim, m_brickPoolDim, m_brickPoolDim, GL_RGBA8, GL_RGBA);
	m_octree_brick_irradiance->SetUnit(2);

	m_octree_node_idx->SetUnit(3);
	m_octree_node_idx->SetImageAccess(GL_READ_ONLY);

	m_octree_node_brick_idx->SetUnit(4);
	m_octree_node_brick_idx->SetImageAccess(GL_READ_WRITE);

	// 给非叶子节点分配Brick
	ShaderPtr brickAllocShader = ShaderLoader::Instance()->LoadShader(Config::Instance()->project_path + "shader/brick_alloc.cmp.glsl");

	brickAllocShader->Use();
	brickAllocShader->SetInteger("u_num", m_numOctreeNoLeafNode);
	brickAllocShader->SetInteger("u_brickPoolDim", m_brickPoolDim);
	brickAllocShader->SetImage("u_octreeNodeIdx", m_octree_node_idx);
	brickAllocShader->SetImage("u_octreeNodeBrickIdx", m_octree_node_brick_idx);
	brickAllocShader->SetImage("u_octreeBrickColor", m_octree_brick_color);
	brickAllocShader->SetImage("u_octreeBrickNormal", m_octree_brick_normal);
	brickAllocShader->SetImage("u_octreeBrickIrradiance", m_octree_brick_irradiance);

	int dataWidth = 1024;
	int dataHeight = (m_numOctreeNoLeafNode + 1023) / dataWidth;
	int allocGroupDimX = dataWidth / 8;
	int allocGroupDimY = (dataHeight + 7) / 8;
	glDispatchCompute(allocGroupDimX, allocGroupDimY, 1);
	m_octree_node_brick_idx->SyncImage();
}

void SVO_VCT::WriteLeafNode()
{
	m_voxel_list_pos->SetUnit(0);
	m_voxel_list_pos->SetImageAccess(GL_READ_ONLY);

	m_voxel_list_color->SetUnit(1);
	m_voxel_list_color->SetImageAccess(GL_READ_ONLY);

	m_voxel_list_normal->SetUnit(2);
	m_voxel_list_normal->SetImageAccess(GL_READ_ONLY);

	m_octree_node_idx->SetUnit(3);
	m_octree_node_idx->SetImageAccess(GL_READ_ONLY);

	m_octree_node_brick_idx->SetUnit(4);
	m_octree_node_brick_idx->SetImageAccess(GL_READ_ONLY);

	m_octree_brick_color->SetUnit(5);
	m_octree_brick_color->SetImageAccess(GL_READ_WRITE);
	
	m_octree_brick_normal->SetUnit(6);
	m_octree_brick_normal->SetImageAccess(GL_READ_WRITE);
	
	m_octree_brick_irradiance->SetUnit(7);
	m_octree_brick_irradiance->SetImageAccess(GL_READ_WRITE);
	
	//Store surface information ( Color, normal, etc. ) into the octree leaf nodes
	ShaderPtr leafStoreShader = ShaderLoader::Instance()->LoadShader(Config::Instance()->project_path + "shader/leaf_store.cmp.glsl");
	
	leafStoreShader->Use();
	leafStoreShader->SetInteger("u_numVoxelFrag", m_numVoxelFrag);
	leafStoreShader->SetInteger("u_octreeLevel", m_octreeLevel);
	leafStoreShader->SetInteger("u_voxelDim", m_voxelSize);
	leafStoreShader->SetImage("u_voxelListPos", m_voxel_list_pos);
	leafStoreShader->SetImage("u_voxelListColor", m_voxel_list_color);
	leafStoreShader->SetImage("u_voxelListNormal", m_voxel_list_normal);
	leafStoreShader->SetImage("u_octreeNodeIdx", m_octree_node_idx);
	leafStoreShader->SetImage("u_octreeNodeBrickIdx", m_octree_node_brick_idx);
	leafStoreShader->SetImage("u_octreeBrickColor", m_octree_brick_color);
	leafStoreShader->SetImage("u_octreeBrickNormal", m_octree_brick_normal);

	int dataWidth = 1024;
	int dataHeight = (m_numVoxelFrag + 1023) / dataWidth;
	int groupDimX = dataWidth / 8;
	int groupDimY = (dataHeight + 7) / 8;
	glDispatchCompute(groupDimX, groupDimY, 1);
	m_octree_brick_color->SyncImage();
}
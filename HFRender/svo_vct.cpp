#include "svo_vct.h"
#include "config.h"

SVO_VCT::SVO_VCT()
{
	m_voxelSize = Config::Instance()->voxelSize;
	m_octreeLevel = log2f(m_voxelSize);
	m_brickPoolDim = Config::Instance()->brickPoolDim;

	m_automic_count = std::make_shared<AutomicBuffer>(0);

	m_voxel_FBO = std::make_shared<Framebuffer>();
	m_voxel_FBO->AttachColorBuffer(std::make_unique<RenderSurface>(m_voxelSize, m_voxelSize, GL_RGB));
}

SVO_VCT::~SVO_VCT()
{

}

void SVO_VCT::SparseVoxelize(World& world)
{
	BuildVoxelList(world);

	BuildOctree();

	AllocBrick();

	WriteLeafNode();

	SpreadLeafBrick();

	BorderTransfer(m_octreeLevel - 1);
}

void SVO_VCT::BuildVoxelList(World& world)
{
	m_automic_count->BindBase(0);

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
	m_automic_count->Sync();
	m_numVoxelFrag = m_automic_count->GetVal();
	m_automic_count->SetVal(0);
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

void SVO_VCT::BuildOctree()
{
	m_numOfLevel.clear();
	m_numOfLevel.push_back(1); //root level has one node

	m_startOfLevel.clear();
	m_startOfLevel.push_back(0);
	m_startOfLevel.push_back(1);

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

	m_octree_node_neigh_x_idx = std::make_shared<TextureBuffer>(init_data.data(), sizeof(GLuint) * totalNode, GL_R32UI);
	m_octree_node_neigh_x_idx->SetUnit(1);

	m_octree_node_neigh_y_idx = std::make_shared<TextureBuffer>(init_data.data(), sizeof(GLuint) * totalNode, GL_R32UI);
	m_octree_node_neigh_y_idx->SetUnit(2);

	m_octree_node_neigh_z_idx = std::make_shared<TextureBuffer>(init_data.data(), sizeof(GLuint) * totalNode, GL_R32UI);
	m_octree_node_neigh_z_idx->SetUnit(3);

	m_voxel_list_pos->SetUnit(4);
	m_voxel_list_pos->SetImageAccess(GL_READ_ONLY);

	m_voxel_list_pos->SetUnit(5);
	m_voxel_list_color->SetImageAccess(GL_READ_ONLY);

	m_voxel_list_pos->SetUnit(6);
	m_voxel_list_normal->SetImageAccess(GL_READ_ONLY);

	m_automic_count->BindBase(0);

	ShaderPtr nodeFlagShader = ShaderLoader::Instance()->LoadShader(Config::Instance()->project_path + "shader/SparseVoxelOctree/node_flag.cmp.glsl");
	ShaderPtr nodeAllocShader = ShaderLoader::Instance()->LoadShader(Config::Instance()->project_path + "shader/SparseVoxelOctree/node_alloc.cmp.glsl");
	ShaderPtr findNeighbourShader = ShaderLoader::Instance()->LoadShader(Config::Instance()->project_path + "shader/SparseVoxelOctree/find_neighbour.cmp.glsl");

	int dataWidth = 1024;
	int dataHeight = (m_numVoxelFrag + 1023) / dataWidth;
	int groupDimX = dataWidth / 8;
	int groupDimY = (dataHeight + 7) / 8;
	//叶子节点在倒数第二层（m_octreeLevel-1）
	for (int i = 0; i < m_octreeLevel - 1; ++i)
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

		int numThread = m_numOfLevel[i];

		if (i > 0)
		{
			//find node neighbour
			findNeighbourShader->Use();
			findNeighbourShader->SetInteger("u_numNode", numThread);
			findNeighbourShader->SetInteger("u_start", m_startOfLevel[i]);
			findNeighbourShader->SetInteger("u_voxelDim", m_voxelSize);
			findNeighbourShader->SetInteger("u_level", i);
			findNeighbourShader->SetInteger("u_levelVoxelSize", m_voxelSize / uint32_t(powf(2, i)));
			findNeighbourShader->SetImage("u_octreeNodeIdx", m_octree_node_idx);
			findNeighbourShader->SetImage("u_octreeNodeNeighXIdx", m_octree_node_neigh_x_idx);
			findNeighbourShader->SetImage("u_octreeNodeNeighYIdx", m_octree_node_neigh_y_idx);
			findNeighbourShader->SetImage("u_octreeNodeNeighZIdx", m_octree_node_neigh_z_idx);
			glDispatchCompute(groupDimX, groupDimY, 1);
			m_octree_node_neigh_x_idx->SyncImage();
		}
		
		//node tile allocation
		nodeAllocShader->Use();
		nodeAllocShader->SetInteger("u_numNode", numThread);
		nodeAllocShader->SetInteger("u_start", m_startOfLevel[i]);
		nodeAllocShader->SetInteger("u_allocStart", m_startOfLevel[i + 1]);
		nodeAllocShader->SetImage("u_octreeNodeIdx", m_octree_node_idx);
		int allocGroupDim = (numThread + 63) / 64;
		glDispatchCompute(allocGroupDim, 1, 1);
		m_octree_node_idx->SyncImage();

		//Get the number of node tiles to allocate in the next level
		m_automic_count->Sync();
		GLuint tileAllocated = m_automic_count->GetVal();
		m_automic_count->SetVal(0);

		//update offsets for next level
		int nodeAllocated = tileAllocated * 8;
		m_numOfLevel.push_back(nodeAllocated); //titleAllocated * 8 is the number of threads we want to launch in the next level
		uint32_t nextStart = m_startOfLevel[i + 1] + nodeAllocated;
		m_startOfLevel.push_back(nextStart);
	}

	//flag leaf node
	nodeFlagShader->Use();
	nodeFlagShader->SetInteger("u_numVoxelFrag", m_numVoxelFrag);
	nodeFlagShader->SetInteger("u_level", m_octreeLevel - 1);
	nodeFlagShader->SetInteger("u_voxelDim", m_voxelSize);
	nodeFlagShader->SetImage("u_voxelListPos", m_voxel_list_pos);
	nodeFlagShader->SetImage("u_octreeNodeIdx", m_octree_node_idx);
	glDispatchCompute(groupDimX, groupDimY, 1);
	m_octree_node_idx->SyncImage();
}

void SVO_VCT::AllocBrick()
{
	int numOctreeNode = m_startOfLevel[m_octreeLevel];

	m_octree_node_brick_idx = std::make_shared<TextureBuffer>(nullptr, sizeof(GLuint) * numOctreeNode, GL_R32UI);
	m_octree_node_brick_idx->SetUnit(0);
	m_octree_node_brick_idx->SetInternalFormat(GL_RGB10_A2UI);

	m_octree_brick_color = std::make_shared<Texture3D>(m_brickPoolDim, m_brickPoolDim, m_brickPoolDim, GL_R32UI, GL_RED_INTEGER);
	m_octree_brick_color->SetUnit(1);

	m_octree_brick_normal = std::make_shared<Texture3D>(m_brickPoolDim, m_brickPoolDim, m_brickPoolDim, GL_R32UI, GL_RED_INTEGER);
	m_octree_brick_normal->SetUnit(2);

	m_octree_brick_irradiance = std::make_shared<Texture3D>(m_brickPoolDim, m_brickPoolDim, m_brickPoolDim, GL_R32UI, GL_RED_INTEGER);
	m_octree_brick_irradiance->SetUnit(3);

	m_octree_node_idx->SetUnit(4);
	m_octree_node_idx->SetImageAccess(GL_READ_ONLY);

	m_automic_count->BindBase(0);

	ShaderPtr brickAllocShader = ShaderLoader::Instance()->LoadShader(Config::Instance()->project_path + "shader/SparseVoxelOctree/brick_alloc.cmp.glsl");
	
	brickAllocShader->Use();
	brickAllocShader->SetInteger("u_numOctreeNode", numOctreeNode);
	brickAllocShader->SetInteger("u_brickPoolDim", m_brickPoolDim);
	brickAllocShader->SetImage("u_octreeNodeIdx", m_octree_node_idx);
	brickAllocShader->SetImage("u_octreeNodeBrickIdx", m_octree_node_brick_idx);
	brickAllocShader->SetImage("u_octreeBrickColor", m_octree_brick_color);
	brickAllocShader->SetImage("u_octreeBrickNormal", m_octree_brick_normal);
	brickAllocShader->SetImage("u_octreeBrickIrradiance", m_octree_brick_irradiance);

	int dataWidth = 1024;
	int dataHeight = (numOctreeNode + 1023) / dataWidth;
	int allocGroupDimX = dataWidth / 8;
	int allocGroupDimY = (dataHeight + 7) / 8;
	glDispatchCompute(allocGroupDimX, allocGroupDimY, 1);
	m_octree_node_brick_idx->SyncImage();
	m_automic_count->SetVal(0);
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
	
	//Store surface information ( Color, normal, etc. ) into the octree leaf nodes
	ShaderPtr leafStoreShader = ShaderLoader::Instance()->LoadShader(Config::Instance()->project_path + "shader/SparseVoxelOctree/leaf_store.cmp.glsl");
	
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

void SVO_VCT::SpreadLeafBrick()
{
	m_octree_node_brick_idx->SetUnit(0);
	m_octree_node_brick_idx->SetImageAccess(GL_READ_ONLY);

	m_octree_brick_color->SetUnit(1);
	m_octree_brick_color->SetImageAccess(GL_READ_WRITE);

	m_octree_brick_normal->SetUnit(2);
	m_octree_brick_normal->SetImageAccess(GL_READ_WRITE);

	ShaderPtr spreadLeafShader = ShaderLoader::Instance()->LoadShader(Config::Instance()->project_path + "shader/SparseVoxelOctree/spread_leaf.cmp.glsl");
	
	int numLeafNode = m_numOfLevel[m_octreeLevel - 1];
	int leafStart = m_startOfLevel[m_octreeLevel - 1];

	int dataWidth = 1024;
	int dataHeight = (numLeafNode + 1023) / dataWidth;
	int allocGroupDimX = dataWidth / 8;
	int allocGroupDimY = (dataHeight + 7) / 8;

	spreadLeafShader->Use();
	spreadLeafShader->SetInteger("u_numLeafNode", numLeafNode);
	spreadLeafShader->SetInteger("u_leafStart", leafStart);
	spreadLeafShader->SetImage("u_octreeNodeBrickIdx", m_octree_node_brick_idx);
	spreadLeafShader->SetImage("u_octreeBrickValue", m_octree_brick_color);
	glDispatchCompute(allocGroupDimX, allocGroupDimY, 1);
	m_octree_brick_color->SyncImage();

	spreadLeafShader->SetImage("u_octreeBrickValue", m_octree_brick_normal);
	glDispatchCompute(allocGroupDimX, allocGroupDimY, 1);
	m_octree_brick_normal->SyncImage();
}

void SVO_VCT::BorderTransfer(int level)
{
	m_octree_node_brick_idx->SetUnit(0);
	m_octree_node_brick_idx->SetImageAccess(GL_READ_ONLY);

	m_octree_brick_color->SetUnit(1);
	m_octree_brick_color->SetImageAccess(GL_READ_WRITE);

	m_octree_brick_normal->SetUnit(2);
	m_octree_brick_normal->SetImageAccess(GL_READ_WRITE);

	m_octree_node_neigh_x_idx->SetUnit(3);
	m_octree_node_neigh_x_idx->SetImageAccess(GL_READ_ONLY);

	m_octree_node_neigh_y_idx->SetUnit(4);
	m_octree_node_neigh_y_idx->SetImageAccess(GL_READ_ONLY);

	m_octree_node_neigh_z_idx->SetUnit(5);
	m_octree_node_neigh_z_idx->SetImageAccess(GL_READ_ONLY);

	ShaderPtr borderTransferShader = ShaderLoader::Instance()->LoadShader(Config::Instance()->project_path + "shader/SparseVoxelOctree/border_transfer.cmp.glsl");

	int numNode = m_numOfLevel[level];
	int start = m_startOfLevel[level];

	int dataWidth = 1024;
	int dataHeight = (numNode + 1023) / dataWidth;
	int allocGroupDimX = dataWidth / 8;
	int allocGroupDimY = (dataHeight + 7) / 8;

	borderTransferShader->Use();
	borderTransferShader->SetInteger("u_numNode", numNode);
	borderTransferShader->SetInteger("u_start", start);
	borderTransferShader->SetImage("u_octreeNodeBrickIdx", m_octree_node_brick_idx);

	//color
	borderTransferShader->SetImage("u_octreeBrickValue", m_octree_brick_color);

	borderTransferShader->SetInteger("u_axis", 0);
	borderTransferShader->SetImage("u_octreeNodeNeighbourIdx", m_octree_node_neigh_x_idx);
	glDispatchCompute(allocGroupDimX, allocGroupDimY, 1);
	m_octree_brick_color->SyncImage();

	borderTransferShader->SetInteger("u_axis", 1);
	borderTransferShader->SetImage("u_octreeNodeNeighbourIdx", m_octree_node_neigh_y_idx);
	glDispatchCompute(allocGroupDimX, allocGroupDimY, 1);
	m_octree_brick_color->SyncImage();

	borderTransferShader->SetInteger("u_axis", 2);
	borderTransferShader->SetImage("u_octreeNodeNeighbourIdx", m_octree_node_neigh_z_idx);
	glDispatchCompute(allocGroupDimX, allocGroupDimY, 1);
	m_octree_brick_color->SyncImage();


	//normal
	borderTransferShader->SetImage("u_octreeBrickValue", m_octree_brick_normal);

	borderTransferShader->SetInteger("u_axis", 0);
	borderTransferShader->SetImage("u_octreeNodeNeighbourIdx", m_octree_node_neigh_x_idx);
	glDispatchCompute(allocGroupDimX, allocGroupDimY, 1);
	m_octree_brick_color->SyncImage();

	borderTransferShader->SetInteger("u_axis", 1);
	borderTransferShader->SetImage("u_octreeNodeNeighbourIdx", m_octree_node_neigh_y_idx);
	glDispatchCompute(allocGroupDimX, allocGroupDimY, 1);
	m_octree_brick_color->SyncImage();

	borderTransferShader->SetInteger("u_axis", 2);
	borderTransferShader->SetImage("u_octreeNodeNeighbourIdx", m_octree_node_neigh_z_idx);
	glDispatchCompute(allocGroupDimX, allocGroupDimY, 1);
	m_octree_brick_color->SyncImage();
}
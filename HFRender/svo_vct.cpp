#include "svo_vct.h"
#include "config.h"
#include "camera.h"
#include "stb_image/stb_image.h"
#include "helpers.h"

SVO_VCT::SVO_VCT()
{
	m_voxelSize = Config::Instance()->voxelSize;
	m_octreeLevel = log2f(m_voxelSize);
	m_shadowMapSize = Config::Instance()->shadowMapSize;

	m_automic_count = std::make_shared<AutomicBuffer>(0);

	m_voxel_FBO = std::make_shared<Framebuffer>();
	m_voxel_FBO->AttachColorBuffer(std::make_unique<RenderSurface>(m_voxelSize, m_voxelSize, GL_RGB));
	assert(m_voxel_FBO->CheckStatus());

	m_shadow_map = std::make_shared<Texture2D>(m_shadowMapSize, m_shadowMapSize, GL_RGB32F, GL_RGB, GL_CLAMP_TO_EDGE, nullptr, GL_FLOAT);
	m_shadow_map_FBO = std::make_shared<Framebuffer>();
	m_shadow_map_FBO->AttachColorTexture(m_shadow_map);
	assert(m_shadow_map_FBO->CheckStatus());
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

	SpreadLeafBrick(m_octree_brick_color, glm::vec4(0));
	SpreadLeafBrick(m_octree_brick_normal, glm::vec4(0.5 * 255, 0.5 * 255, 0.5 * 255, 0));

	BorderTransfer(m_octreeLevel - 1, m_octree_brick_color);
	BorderTransfer(m_octreeLevel - 1, m_octree_brick_normal);

	for (int level = m_octreeLevel - 2; level >= 0; level--)
	{
		Mipmap(level, m_octree_brick_color, glm::vec4(0));
		Mipmap(level, m_octree_brick_normal, glm::vec4(0.5 * 255, 0.5 * 255, 0.5 * 255, 0));
		if (level > 0)
		{
			BorderTransfer(level, m_octree_brick_color);
			BorderTransfer(level, m_octree_brick_normal);
		}
	}
}

void SVO_VCT::LightUpdate(World& world)
{
	ShadowMap(world);
	return;
	LightInjection(world);

	SpreadLeafBrick(m_octree_brick_irradiance, glm::vec4(0));
	BorderTransfer(m_octreeLevel - 1, m_octree_brick_irradiance);

	for (int level = m_octreeLevel - 2; level >= 0; level--)
	{
		Mipmap(level, m_octree_brick_irradiance, glm::vec4(0));
		if (level > 0)
		{
			BorderTransfer(level, m_octree_brick_irradiance);
		}
	}
}

void SVO_VCT::BuildVoxelList(World& world)
{
	std::cout << "BuildVoxelList" << std::endl;

	glm::mat4 orth = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
	Camera camera;
	camera.SetPosition(glm::vec3(0));

	camera.SetForward(glm::vec3(-1, 0, 0));
	glm::mat4 vpX = orth * camera.GetViewMatrix();

	camera.SetForward(glm::vec3(0, -1, 0));
	glm::mat4 vpY = orth * camera.GetViewMatrix();

	camera.SetForward(glm::vec3(0, 0, -1));
	glm::mat4 vpZ = orth * camera.GetViewMatrix();

	const std::unordered_map<std::string, IEntityPtr>& entities = world.GetEntities();
	for (auto& it : entities)
	{
		ModelEntity* modelEntity = dynamic_cast<ModelEntity*>(it.second.get());
		const ModelDataPtr& modelData = modelEntity->GetModelData();
		const TinyobjMaterialPtr& mat_t = modelData->GetMaterial();

		ParamTable params = {
			{"u_VPx", vpX},
			{"u_VPy", vpY},
			{"u_VPz", vpZ},
			{"u_voxelSize", Config::Instance()->voxelSize},
			{"u_bStore", 0}
		};

		int width, height, nrChannels;
		std::string image_path = Config::Instance()->project_path + "resource/model/sponza/" + mat_t->diffuse_texname;
		unsigned char* data = stbi_load(image_path.c_str(), &width, &height, &nrChannels, 0);
		Texture2DPtr diffuseMap;
		if (nrChannels == 3)
		{
			diffuseMap = std::make_shared<Texture2D>(width, height, data);
		}
		else if (nrChannels == 4)
		{
			diffuseMap = std::make_shared<Texture2D>(width, height, GL_RGBA8, GL_RGBA, GL_REPEAT, data, GL_UNSIGNED_BYTE);
		}
		else
		{
			assert(0);
		}
		TextureParamTable texture_param = {
			{"u_diffuseMap", diffuseMap}
		};

		MaterialPtr material = Material::CreateMaterial(Config::Instance()->project_path + "shader/SparseVoxelOctree/voxelize_list.vert",
			Config::Instance()->project_path + "shader/SparseVoxelOctree/voxelize_list.frag",
			Config::Instance()->project_path + "shader/SparseVoxelOctree/voxelize_list.geom", std::move(params), std::move(texture_param));
		it.second->SetMaterial(material);
	}

	ViewContext voxel_vc;
	voxel_vc.SetColorMask(glm::bvec4(false));
	voxel_vc.SetDepthStates(false, false, GL_LESS);
	voxel_vc.SetCullFace(false, GL_BACK);
	voxel_vc.SetBlend(false);
	voxel_vc.SetFramebuffer(m_voxel_FBO);

	glViewport(0, 0, m_voxelSize, m_voxelSize);
	world.CommitRenderContext(voxel_vc);
	m_automic_count->BindBase(0);

	// Obtain number of voxel fragments
	voxel_vc.FlushRenderContext(false);
	m_automic_count->Sync();
	m_numVoxelFrag = m_automic_count->GetVal();
	m_automic_count->SetVal(0);
	m_automic_count->Sync();
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
	std::cout << "BuildOctree" << std::endl;

	m_numOfLevel.clear();
	m_numOfLevel.push_back(1); //root level has one node

	m_startOfLevel.clear();
	m_startOfLevel.push_back(0);
	m_startOfLevel.push_back(1);

	//Calculate the maximum possilbe node number
	int totalNode = 1;
	int nTmp = 1;
	//叶子节点在倒数第二层（m_octreeLevel-1）
	for (int i = 1; i <= m_octreeLevel - 1; ++i)
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

	m_automic_count->SetVal(0);
	m_automic_count->Sync();
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
		m_automic_count->Sync();

		//update offsets for next level
		int nodeAllocated = tileAllocated * 8;
		m_numOfLevel.push_back(nodeAllocated); //titleAllocated * 8 is the number of threads we want to launch in the next level
		uint32_t nextStart = m_startOfLevel[i + 1] + nodeAllocated;
		m_startOfLevel.push_back(nextStart);

		std::cout << "level " << i + 1 << " : " << nodeAllocated << std::endl;
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

	//find leaf node neighbour
	findNeighbourShader->Use();
	findNeighbourShader->SetInteger("u_numNode", m_numOfLevel[m_octreeLevel - 1]);
	findNeighbourShader->SetInteger("u_start", m_startOfLevel[m_octreeLevel - 1]);
	findNeighbourShader->SetInteger("u_voxelDim", m_voxelSize);
	findNeighbourShader->SetInteger("u_level", m_octreeLevel - 1);
	findNeighbourShader->SetInteger("u_levelVoxelSize", m_voxelSize / uint32_t(powf(2, m_octreeLevel - 1)));
	findNeighbourShader->SetImage("u_octreeNodeIdx", m_octree_node_idx);
	findNeighbourShader->SetImage("u_octreeNodeNeighXIdx", m_octree_node_neigh_x_idx);
	findNeighbourShader->SetImage("u_octreeNodeNeighYIdx", m_octree_node_neigh_y_idx);
	findNeighbourShader->SetImage("u_octreeNodeNeighZIdx", m_octree_node_neigh_z_idx);
	glDispatchCompute(groupDimX, groupDimY, 1);
	m_octree_node_neigh_x_idx->SyncImage();
}

void SVO_VCT::AllocBrick()
{
	std::cout << "AllocBrick" << std::endl;

	int numOctreeNode = m_startOfLevel[m_octreeLevel];

	m_octree_node_brick_idx = std::make_shared<TextureBuffer>(nullptr, sizeof(GLuint) * numOctreeNode, GL_R32UI);
	m_octree_node_brick_idx->SetUnit(0);
	m_octree_node_brick_idx->SetInternalFormat(GL_RGB10_A2UI);

	m_brickPoolDim = ceilf(powf(numOctreeNode, 1.0 / 3)) * 3;
	std::cout << "brickPoolDim: " << m_brickPoolDim << std::endl;
	m_octree_brick_color = std::make_shared<Texture3D>(m_brickPoolDim, m_brickPoolDim, m_brickPoolDim, GL_R32UI, GL_RED_INTEGER);
	m_octree_brick_color->SetUnit(1);

	m_octree_brick_normal = std::make_shared<Texture3D>(m_brickPoolDim, m_brickPoolDim, m_brickPoolDim, GL_R32UI, GL_RED_INTEGER);
	m_octree_brick_normal->SetUnit(2);

	m_octree_brick_irradiance = std::make_shared<Texture3D>(m_brickPoolDim, m_brickPoolDim, m_brickPoolDim, GL_R32UI, GL_RED_INTEGER);
	m_octree_brick_irradiance->SetUnit(3);

	m_octree_node_idx->SetUnit(4);
	m_octree_node_idx->SetImageAccess(GL_READ_ONLY);

	m_automic_count->SetVal(0);
	m_automic_count->Sync();
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

	m_automic_count->Sync();
	GLuint allocBrickSize = m_automic_count->GetVal();
	std::cout << "allocBrickSize: " << allocBrickSize << std::endl;
	m_automic_count->SetVal(0);
	m_automic_count->Sync();
}

void SVO_VCT::WriteLeafNode()
{
	std::cout << "WriteLeafNode" << std::endl;

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

void SVO_VCT::SpreadLeafBrick(const Texture3DPtr& octree_brick, const glm::vec4& empty_color)
{
	std::cout << "SpreadLeafBrick" << std::endl;

	m_octree_node_idx->SetUnit(0);
	m_octree_node_idx->SetImageAccess(GL_READ_ONLY);

	m_octree_node_brick_idx->SetUnit(1);
	m_octree_node_brick_idx->SetImageAccess(GL_READ_ONLY);

	octree_brick->SetUnit(2);
	octree_brick->SetImageAccess(GL_READ_WRITE);

	ShaderPtr spreadLeafShader = ShaderLoader::Instance()->LoadShader(Config::Instance()->project_path + "shader/SparseVoxelOctree/spread_leaf.cmp.glsl");
	
	int numLeafNode = m_numOfLevel[m_octreeLevel - 1];
	int leafStart = m_startOfLevel[m_octreeLevel - 1];
	int emptyValue = convVec4ToRGBA8(empty_color);

	int dataWidth = 1024;
	int dataHeight = (numLeafNode + 1023) / dataWidth;
	int allocGroupDimX = dataWidth / 8;
	int allocGroupDimY = (dataHeight + 7) / 8;

	spreadLeafShader->Use();
	spreadLeafShader->SetInteger("u_numLeafNode", numLeafNode);
	spreadLeafShader->SetInteger("u_leafStart", leafStart);
	spreadLeafShader->SetInteger("u_emptyValue", emptyValue);
	spreadLeafShader->SetImage("u_octreeNodeIdx", m_octree_node_idx);
	spreadLeafShader->SetImage("u_octreeNodeBrickIdx", m_octree_node_brick_idx);
	spreadLeafShader->SetImage("u_octreeBrickValue", octree_brick);
	glDispatchCompute(allocGroupDimX, allocGroupDimY, 1);
	octree_brick->SyncImage();
}

void SVO_VCT::BorderTransfer(int level, const Texture3DPtr& octree_brick)
{
	std::cout << "BorderTransfer" << std::endl;

	m_octree_node_brick_idx->SetUnit(0);
	m_octree_node_brick_idx->SetImageAccess(GL_READ_ONLY);

	octree_brick->SetUnit(1);
	octree_brick->SetImageAccess(GL_READ_WRITE);

	m_octree_node_neigh_x_idx->SetUnit(2);
	m_octree_node_neigh_x_idx->SetImageAccess(GL_READ_ONLY);

	m_octree_node_neigh_y_idx->SetUnit(3);
	m_octree_node_neigh_y_idx->SetImageAccess(GL_READ_ONLY);

	m_octree_node_neigh_z_idx->SetUnit(4);
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
	borderTransferShader->SetImage("u_octreeBrickValue", octree_brick);

	borderTransferShader->SetInteger("u_axis", 0);
	borderTransferShader->SetImage("u_octreeNodeNeighbourIdx", m_octree_node_neigh_x_idx);
	glDispatchCompute(allocGroupDimX, allocGroupDimY, 1);
	octree_brick->SyncImage();

	borderTransferShader->SetInteger("u_axis", 1);
	borderTransferShader->SetImage("u_octreeNodeNeighbourIdx", m_octree_node_neigh_y_idx);
	glDispatchCompute(allocGroupDimX, allocGroupDimY, 1);
	octree_brick->SyncImage();

	borderTransferShader->SetInteger("u_axis", 2);
	borderTransferShader->SetImage("u_octreeNodeNeighbourIdx", m_octree_node_neigh_z_idx);
	glDispatchCompute(allocGroupDimX, allocGroupDimY, 1);
	octree_brick->SyncImage();
}

void SVO_VCT::Mipmap(int level, const Texture3DPtr& octree_brick, const glm::vec4& empty_color)
{
	std::cout << "Mipmap" << std::endl;

	m_octree_node_idx->SetUnit(0);
	m_octree_node_idx->SetImageAccess(GL_READ_ONLY);

	m_octree_node_brick_idx->SetUnit(1);
	m_octree_node_brick_idx->SetImageAccess(GL_READ_ONLY);

	octree_brick->SetUnit(2);
	octree_brick->SetImageAccess(GL_READ_WRITE);

	ShaderPtr mipmapShader = ShaderLoader::Instance()->LoadShader(Config::Instance()->project_path + "shader/SparseVoxelOctree/mipmap.cmp.glsl");

	int numNode = m_numOfLevel[level];
	int start = m_startOfLevel[level];

	std::cout << "level " << level << " : " << numNode << " " << start << std::endl;

	int dataWidth = 1024;
	int dataHeight = (numNode + 1023) / dataWidth;
	int allocGroupDimX = dataWidth / 8;
	int allocGroupDimY = (dataHeight + 7) / 8;

	mipmapShader->Use();
	mipmapShader->SetInteger("u_numNode", numNode);
	mipmapShader->SetInteger("u_start", start);
	mipmapShader->SetInteger("u_level", level);
	mipmapShader->SetInteger("u_octreeLevel", m_octreeLevel);
	mipmapShader->SetImage("u_octreeNodeIdx", m_octree_node_idx);
	mipmapShader->SetImage("u_octreeNodeBrickIdx", m_octree_node_brick_idx);
	mipmapShader->SetImage("u_octreeBrickValue", octree_brick);
	mipmapShader->SetVector4f("u_emptyColor", empty_color);
	glDispatchCompute(allocGroupDimX, allocGroupDimY, 1);
	octree_brick->SyncImage();
}

void SVO_VCT::ShadowMap(World& world)
{
	std::cout << "ShadowMap" << std::endl;

	const std::vector<DirectionLight>& lights = world.GetLights();

	MaterialPtr smMaterial = Material::CreateMaterial(Config::Instance()->project_path + "shader/SparseVoxelOctree/shadow_map.vert",
		Config::Instance()->project_path + "shader/SparseVoxelOctree/shadow_map.frag", "", {});

	world.SetMaterial(smMaterial);

	Camera camera;
	camera.SetPosition(glm::vec3(0));
	camera.SetForward(lights[0].GetDirection());
	camera.SetProjMatrix(lights[0].GetLightProjMatrix());

	//ViewContext sm_vc;
	m_sm_vc.SetColorMask(glm::bvec4(true));
	m_sm_vc.SetDepthStates(true, true, GL_LESS);
	m_sm_vc.SetCullFace(false, GL_BACK);
	m_sm_vc.SetBlend(false);
	m_sm_vc.SetFramebuffer(m_shadow_map_FBO);

	glViewport(0, 0, m_shadowMapSize, m_shadowMapSize);

	camera.FillViewContext(m_sm_vc);
	world.CommitRenderContext(m_sm_vc);
	m_sm_vc.FlushRenderContext(true);
}

void SVO_VCT::LightInjection(World& world)
{
	std::cout << "LightInjection" << std::endl;

	m_shadow_map->SetUnit(0);

	m_octree_node_idx->SetUnit(1);
	m_octree_node_idx->SetImageAccess(GL_READ_ONLY);

	m_octree_node_brick_idx->SetUnit(2);
	m_octree_node_brick_idx->SetImageAccess(GL_READ_ONLY);

	m_octree_brick_color->SetUnit(3);
	m_octree_brick_color->SetImageAccess(GL_READ_ONLY);

	m_octree_brick_irradiance->SetUnit(4);
	m_octree_brick_irradiance->SetImageAccess(GL_READ_WRITE);

	const std::vector<DirectionLight>& lights = world.GetLights();

	ShaderPtr lightShader = ShaderLoader::Instance()->LoadShader(Config::Instance()->project_path + "shader/SparseVoxelOctree/light_injection.cmp.glsl");

	int allocGroupDimX = (m_shadowMapSize + 7) / 8;
	int allocGroupDimY = (m_shadowMapSize + 7) / 8;

	lightShader->Use();
	lightShader->SetInteger("u_voxelDim", m_voxelSize);
	lightShader->SetInteger("u_octreeLevel", m_octreeLevel);
	lightShader->SetVector3f("u_light.direction", lights[0].GetDirection());
	lightShader->SetVector3f("u_light.color", lights[0].GetColor());
	lightShader->SetImage("u_octreeNodeIdx", m_octree_node_idx);
	lightShader->SetImage("u_octreeNodeBrickIdx", m_octree_node_brick_idx);
	lightShader->SetImage("u_octreeBrickColor", m_octree_brick_color);
	lightShader->SetImage("u_octreeBrickIrradiance", m_octree_brick_irradiance);
	lightShader->SetTexture("u_shadowMap", m_shadow_map);

	glDispatchCompute(allocGroupDimX, allocGroupDimY, 1);
	m_octree_brick_irradiance->SyncImage();
}

void SVO_VCT::Check()
{
	std::cout << "Check" << std::endl;

	int numOctreeNode = m_startOfLevel[m_octreeLevel];

	m_octree_node_brick_idx->SetUnit(0);
	m_octree_node_brick_idx->SetImageAccess(GL_READ_ONLY);

	m_automic_count->SetVal(0);
	m_automic_count->Sync();
	m_automic_count->BindBase(0);

	ShaderPtr checkShader = ShaderLoader::Instance()->LoadShader(Config::Instance()->project_path + "shader/SparseVoxelOctree/check_brick_idx.cmp.glsl");

	checkShader->Use();
	checkShader->SetInteger("u_numOctreeNode", numOctreeNode);
	checkShader->SetInteger("u_brickPoolDim", m_brickPoolDim);
	checkShader->SetImage("u_octreeNodeBrickIdx", m_octree_node_brick_idx);

	int dataWidth = 1024;
	int dataHeight = (numOctreeNode + 1023) / dataWidth;
	int allocGroupDimX = dataWidth / 8;
	int allocGroupDimY = (dataHeight + 7) / 8;
	glDispatchCompute(allocGroupDimX, allocGroupDimY, 1);
	m_octree_node_brick_idx->SyncImage();

	m_automic_count->Sync();
	GLuint errorNum = m_automic_count->GetVal();
	std::cout << "brickIdx ErrorNum: " << errorNum << std::endl;
	m_automic_count->SetVal(0);
	m_automic_count->Sync();
}
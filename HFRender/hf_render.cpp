#include "hf_render.h"
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>
#include "helpers.h"

const char* glsl_version = "#version 450 core";
HFRender* HFRender::s_inst = new HFRender();

void HFRender::ProcessInput()
{
	if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(m_window, true);
		return;
	}

	float current_time = glfwGetTime();
	float delta_time = current_time - m_last_time;
	m_last_time = current_time;

	// move camera
	float distance = m_camera_move_speed * delta_time;

	if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
		m_camera.SetPosition(m_camera.GetPosition() + m_camera.GetForward() * distance);
	else if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
		m_camera.SetPosition(m_camera.GetPosition() - m_camera.GetForward() * distance);

	if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
		m_camera.SetPosition(m_camera.GetPosition() - m_camera.GetRight() * distance);
	else if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
		m_camera.SetPosition(m_camera.GetPosition() + m_camera.GetRight() * distance);

	if (glfwGetKey(m_window, GLFW_KEY_Q) == GLFW_PRESS)
		m_camera.SetPosition(m_camera.GetPosition() + glm::vec3(0.f, distance, 0.f));
	else if (glfwGetKey(m_window, GLFW_KEY_E) == GLFW_PRESS)
		m_camera.SetPosition(m_camera.GetPosition() - glm::vec3(0.f, distance, 0.f));
}

void HFRender::ProcessMouseMovement(float xpos, float ypos)
{
	if (std::isnan(m_last_mouse_x) || std::isnan(m_last_mouse_y))
	{
		m_last_mouse_x = xpos;
		m_last_mouse_y = ypos;
	}

	float xoffset = m_last_mouse_x - xpos;
	float yoffset = m_last_mouse_y - ypos;

	m_last_mouse_x = xpos;
	m_last_mouse_y = ypos;

	xoffset *= m_mouse_sensitivity;
	yoffset *= m_mouse_sensitivity;

	// rotate the forward vector
	glm::vec3 forward = m_camera.GetForward();
	glm::vec3 right = m_camera.GetRight();
	forward = glm::rotateY(forward, xoffset);
	forward = glm::rotate(forward, yoffset, right);
	m_camera.SetForward(forward);
}

void HFRender::ProcessMouseScroll(float yoffset)
{
	float distance = m_camera_move_speed * yoffset;
	m_camera.SetPosition(m_camera.GetPosition() + m_camera.GetForward() * distance);
}

void HFRender::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	Config::Instance()->width = width;
	Config::Instance()->height = height;
	glViewport(0, 0, width, height);
}

void HFRender::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	HFRender::Instance()->ProcessMouseMovement(xpos, ypos);
}

void HFRender::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	HFRender::Instance()->ProcessMouseScroll(yoffset);
}

bool HFRender::InitGlfw()
{
	// 初始化GLFW
	if (glfwInit() != GLFW_TRUE)
	{
		printf("InitGlfw failed!");
		return false;
	}

	// 配置GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);  // 指定OpenGL主版本号
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);  // 指定OpenGL次版本号
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 指定核心模式
	return true;
}

bool HFRender::InitGlad()
{
	// 初始化GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		printf("InitGlad failed!");
		return false;
	}
	return true;
}

bool HFRender::Init(int width, int height)
{
	Config::Instance()->width = width;
	Config::Instance()->height = height;

	if (!InitGlfw())
	{
		return false;
	}

	// 创建窗口
	m_window = glfwCreateWindow(width, height, "HFRender", NULL, NULL);
	if (!m_window)
	{
		printf("Failed to create GLFW window");
		return false;
	}

	glfwMakeContextCurrent(m_window);
	glfwSwapInterval(1);
	glfwSetFramebufferSizeCallback(m_window, FramebufferSizeCallback);
	glfwSetCursorPosCallback(m_window, CursorPosCallback);
	glfwSetScrollCallback(m_window, ScrollCallback);

	// tell GLFW to capture our mouse
	//glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!InitGlad())
	{
		printf("Fail to init glad");
		return false;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	return true;
}

void HFRender::Destroy()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	if (m_window)
	{
		glfwDestroyWindow(m_window);
		m_window = nullptr;
	}
	glfwTerminate();
}

void HFRender::RenderImGui()
{
	//ImGui::ShowDemoWindow();
	ImGui::Begin("Config");

	ImGui::LabelText("label", "Value");
	const char* items[] = { "Forward", "Deferred" };
	ImGui::Combo("Render mode", reinterpret_cast<int*>(&Config::Instance()->render_mode), items, IM_ARRAYSIZE(items));

	ImGui::End();
}

bool HFRender::Render()
{
	glViewport(0, 0, Config::Instance()->width, Config::Instance()->height);

	m_world.AddModelEntity(Config::Instance()->project_path + "resource/model/cornell.obj", glm::mat4(1), "cornell", Material::GetDefaultMaterial());

	m_camera.SetPosition(glm::vec3(0, 0, 3));
	m_camera.SetForward(glm::vec3(0, 0, -1));

	ViewContext vc;
	//vc.SetCullFace(true, GL_BACK);
	m_world.CommitRenderContext(vc);

	while (!glfwWindowShouldClose(m_window))
	{
		glfwPollEvents();
		ProcessInput();

		// feed inputs to dear imgui, start new frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// render world
		m_camera.FillViewContext(vc);
		vc.FlushRenderContext(false);

		// render your GUI
		RenderImGui();

		// Render dear imgui into screen
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(m_window);
	}

	return true;
}

void HFRender::Voxelize()
{
	int voxelSize = Config::Instance()->voxelSize;
	m_texture3D = std::make_shared<Texture3D>(voxelSize, voxelSize, voxelSize, GL_RGBA8, GL_RGBA, true);
	m_texture3D->SetUnit(0);
	FramebufferPtr voxel_FBO = std::make_shared<Framebuffer>();
	voxel_FBO->AttachColorBuffer(std::make_unique<RenderSurface>(voxelSize, voxelSize, GL_RGB));
	assert(voxel_FBO->CheckStatus());

	MaterialPtr bunny_material = Material::GetVoxelMaterial(m_texture3D, glm::vec3(1, 1, 1));
	Transform bunny_transform(glm::vec3(0, -0.50f, 0), glm::mat4(1), glm::vec3(0.5));
	m_voxel_world.AddModelEntity(Config::Instance()->project_path + "resource/model/bunny.obj", bunny_transform.GetMatrix(), "bunny", bunny_material);

	Transform cornell_trans(glm::vec3(0), glm::mat4(1), glm::vec3(0.99));

	MaterialPtr cornell_left_material = Material::GetVoxelMaterial(m_texture3D, glm::vec3(1, 0, 0));
	m_voxel_world.AddModelEntity(Config::Instance()->project_path + "resource/model/left_wall.obj", cornell_trans.GetMatrix(), "left_wall", cornell_left_material);

	MaterialPtr cornell_right_material = Material::GetVoxelMaterial(m_texture3D, glm::vec3(0, 0, 1));
	m_voxel_world.AddModelEntity(Config::Instance()->project_path + "resource/model/right_wall.obj", cornell_trans.GetMatrix(), "right_wall", cornell_right_material);

	MaterialPtr cornell_other_material = Material::GetVoxelMaterial(m_texture3D, glm::vec3(1, 1, 1));
	m_voxel_world.AddModelEntity(Config::Instance()->project_path + "resource/model/floor_roof.obj", cornell_trans.GetMatrix(), "floor_roof", cornell_other_material);

	glViewport(0, 0, voxelSize, voxelSize);

	Camera camera;
	camera.SetForward(glm::vec3(0, 0, -1));
	camera.SetProjMatrix(glm::ortho(-1, 1, -1, 1, -1, 1));

	ViewContext voxel_vc;
	voxel_vc.SetColorMask(glm::bvec4(false));
	voxel_vc.SetDepthStates(false, false, GL_LESS);
	voxel_vc.SetCullFace(false, GL_BACK);
	voxel_vc.SetBlend(false);
	voxel_vc.SetFramebuffer(voxel_FBO);

	camera.FillViewContext(voxel_vc);
	m_voxel_world.CommitRenderContext(voxel_vc);
	voxel_vc.FlushRenderContext(true);

	m_texture3D->GenerateMipmap();

	//std::vector<GLfloat> data(4 * voxelSize * voxelSize * voxelSize);
	//m_texture3D->ReadTextureData(&data[0], GL_FLOAT);
	//dump_3D_data(&data[0], 4, voxelSize, voxelSize, voxelSize, "C:\\Users\\wanglingye\\wly\\project\\HFRender\\out\\texture3D.xyz");
}

void HFRender::RenderVoxel()
{
	int voxelSize = Config::Instance()->voxelSize;
	int width = Config::Instance()->width;
	int height = Config::Instance()->height;

	ParamTable params2 = {
		{"pointSize", float(height) / float(voxelSize)}
	};

	TextureParamTable texture_param2 = {
		{"texture3D", m_texture3D}
	};
	MaterialPtr volumeVisualizationMaterial = Material::CreateMaterial(Config::Instance()->project_path + "shader/Visualization/volume_visualization.vert",
		Config::Instance()->project_path + "shader/Visualization/volume_visualization.frag", "", std::move(params2), std::move(texture_param2));

	VolumePtr volume = std::make_unique<Volume>(glm::vec3(-1), 2.0f / voxelSize, voxelSize, voxelSize, voxelSize, volumeVisualizationMaterial);
	m_world.AddEntity("volume_visualization", std::move(volume));

	m_camera.SetPosition(glm::vec3(0, 0, 2));
	m_camera.SetForward(glm::vec3(0, 0, -1));

	glViewport(0, 0, Config::Instance()->width, Config::Instance()->height);

	ViewContext volume_vc;
	volume_vc.SetDepthStates(true, true, GL_LESS);
	m_world.CommitRenderContext(volume_vc, "volume_visualization");

	glEnable(GL_PROGRAM_POINT_SIZE);
	while (!glfwWindowShouldClose(m_window))
	{
		glfwPollEvents();
		ProcessInput();

		m_camera.FillViewContext(volume_vc);
		volume_vc.FlushRenderContext(false);

		glfwSwapBuffers(m_window);
	}
}

void HFRender::VoxelConeTrace()
{
	int voxelSize = Config::Instance()->voxelSize;
	int width = Config::Instance()->width;
	int height = Config::Instance()->height;

	MaterialPtr bunny_material = Material::GetVCTSpecular(m_texture3D, glm::vec3(1, 1, 1));
	Transform transform(glm::vec3(0, -0.50f, 0), glm::mat4(1), glm::vec3(0.5));
	m_world.AddModelEntity(Config::Instance()->project_path + "resource/model/bunny.obj", transform.GetMatrix(), "bunny", bunny_material);

	Transform cornell_trans(glm::vec3(0), glm::mat4(1), glm::vec3(0.99));

	MaterialPtr cornell_left_material = Material::GetVCTDiffuse(m_texture3D, glm::vec3(1, 0, 0));
	m_world.AddModelEntity(Config::Instance()->project_path + "resource/model/left_wall.obj", cornell_trans.GetMatrix(), "left_wall", cornell_left_material);

	MaterialPtr cornell_right_material = Material::GetVCTDiffuse(m_texture3D, glm::vec3(0, 0, 1));
	m_world.AddModelEntity(Config::Instance()->project_path + "resource/model/right_wall.obj", cornell_trans.GetMatrix(), "right_wall", cornell_right_material);

	MaterialPtr cornell_other_material = Material::GetVCTDiffuse(m_texture3D, glm::vec3(1, 1, 1));
	m_world.AddModelEntity(Config::Instance()->project_path + "resource/model/floor_roof.obj", cornell_trans.GetMatrix(), "floor_roof", cornell_other_material);

	ViewContext vc;
	vc.SetDepthStates(true, true, GL_LESS);
	m_world.CommitRenderContext(vc);

	m_camera.SetPosition(glm::vec3(0, 0, 3));
	m_camera.SetForward(glm::vec3(0, 0, -1));

	glViewport(0, 0, Config::Instance()->width, Config::Instance()->height);
	while (!glfwWindowShouldClose(m_window))
	{
		glfwPollEvents();
		ProcessInput();

		m_camera.FillViewContext(vc);
		vc.FlushRenderContext(false);

		glfwSwapBuffers(m_window);
	}
}

void HFRender::BuildVoxelList()
{
	int voxelSize = Config::Instance()->voxelSize;

	MaterialPtr bunny_material = Material::GetVoxelListMaterial(glm::vec3(1, 1, 1));
	Transform bunny_transform(glm::vec3(0, -0.50f, 0), glm::mat4(1), glm::vec3(0.5));
	m_voxel_world.AddModelEntity(Config::Instance()->project_path + "resource/model/bunny.obj", bunny_transform.GetMatrix(), "bunny", bunny_material);

	Transform cornell_trans(glm::vec3(0), glm::mat4(1), glm::vec3(0.99));
	MaterialPtr cornell_left_material = Material::GetVoxelListMaterial(glm::vec3(1, 0, 0));
	m_voxel_world.AddModelEntity(Config::Instance()->project_path + "resource/model/left_wall.obj", cornell_trans.GetMatrix(), "left_wall", cornell_left_material);

	MaterialPtr cornell_right_material = Material::GetVoxelListMaterial(glm::vec3(0, 0, 1));
	m_voxel_world.AddModelEntity(Config::Instance()->project_path + "resource/model/right_wall.obj", cornell_trans.GetMatrix(), "right_wall", cornell_right_material);

	MaterialPtr cornell_other_material = Material::GetVoxelListMaterial(glm::vec3(1, 1, 1));
	m_voxel_world.AddModelEntity(Config::Instance()->project_path + "resource/model/floor_roof.obj", cornell_trans.GetMatrix(), "floor_roof", cornell_other_material);

	AutomicBufferPtr automic_buffer = std::make_shared<AutomicBuffer>(0);
	automic_buffer->BindBase(0);

	FramebufferPtr voxel_FBO = std::make_shared<Framebuffer>();
	voxel_FBO->AttachColorBuffer(std::make_unique<RenderSurface>(voxelSize, voxelSize, GL_RGB));
	assert(voxel_FBO->CheckStatus());

	ViewContext voxel_vc;
	voxel_vc.SetColorMask(glm::bvec4(false));
	voxel_vc.SetDepthStates(false, false, GL_LESS);
	voxel_vc.SetCullFace(false, GL_BACK);
	voxel_vc.SetBlend(false);
	voxel_vc.SetFramebuffer(voxel_FBO);

	glViewport(0, 0, voxelSize, voxelSize);
	m_voxel_world.CommitRenderContext(voxel_vc);

	// Obtain number of voxel fragments
	voxel_vc.FlushRenderContext(false);
	automic_buffer->Sync();
	m_numVoxelFrag = automic_buffer->GetVal();
	automic_buffer->SetVal(0);
	std::cout << "Number of Entries in Voxel Fragment List: " << m_numVoxelFrag << std::endl;

	// Create buffers for voxel fragment list
	m_voxel_pos = std::make_shared<TextureBuffer>(nullptr, sizeof(GLuint) * m_numVoxelFrag, GL_R32UI);
	m_voxel_pos->SetUnit(0);
	m_voxel_pos->SetInternalFormat(GL_RGB10_A2UI);
	m_voxel_kd = std::make_shared<TextureBuffer>(nullptr, sizeof(GLuint) * m_numVoxelFrag, GL_RGBA8);
	m_voxel_kd->SetUnit(1);

	bunny_material->SetParam("u_bStore", 1);
	bunny_material->SetImageParam("u_voxelPos", m_voxel_pos);
	bunny_material->SetImageParam("u_voxelKd", m_voxel_kd);

	cornell_left_material->SetParam("u_bStore", 1);
	cornell_left_material->SetImageParam("u_voxelPos", m_voxel_pos);
	cornell_left_material->SetImageParam("u_voxelKd", m_voxel_kd);

	cornell_right_material->SetParam("u_bStore", 1);
	cornell_right_material->SetImageParam("u_voxelPos", m_voxel_pos);
	cornell_right_material->SetImageParam("u_voxelKd", m_voxel_kd);

	cornell_other_material->SetParam("u_bStore", 1);
	cornell_other_material->SetImageParam("u_voxelPos", m_voxel_pos);
	cornell_other_material->SetImageParam("u_voxelKd", m_voxel_kd);

	//Voxelize the scene again, this time store the data in the voxel fragment list
	voxel_vc.FlushRenderContext(false);
	m_voxel_pos->SyncImage();

	//std::vector<GLuint> data(m_numVoxelFrag);
	//m_voxel_pos->ReadTextureData(&data[0], sizeof(GLuint) * m_numVoxelFrag);
	//dump_buffer_data(&data[0], m_numVoxelFrag, "C:\\Users\\wanglingye\\wly\\project\\HFRender\\out\\voxel_list.xyz");
}

void HFRender::BuildSVO()
{
	int voxelSize = Config::Instance()->voxelSize;
	int octreeLevel = Config::Instance()->octreeLevel;

	std::vector<unsigned int> allocList; //the vector records the number of nodes in each tree level
	allocList.push_back(1); //root level has one node

	//Calculate the maximum possilbe node number
	int totalNode = 1;
	int nTmp = 1;
	for (int i = 1; i <= octreeLevel; ++i)
	{
		nTmp *= 8;
		totalNode += nTmp;
	}

	m_octree_idx = std::make_shared<TextureBuffer>(nullptr, sizeof(GLuint) * totalNode, GL_R32UI);
	m_octree_idx->SetUnit(2);
	m_octree_kd = std::make_shared<TextureBuffer>(nullptr, sizeof(GLuint) * totalNode, GL_R32UI);
	m_octree_kd->SetUnit(3);

	m_voxel_pos->SetImageAccess(GL_READ_ONLY);
	m_voxel_kd->SetImageAccess(GL_READ_ONLY);

	AutomicBufferPtr allocCounter = std::make_shared<AutomicBuffer>(0);
	allocCounter->BindBase(0);

	ShaderPtr nodeFlagShader = ShaderLoader::Instance()->LoadShader(Config::Instance()->project_path + "shader/node_flag.cmp.glsl");
	ShaderPtr nodeAllocShader = ShaderLoader::Instance()->LoadShader(Config::Instance()->project_path + "shader/node_alloc.cmp.glsl");
	ShaderPtr nodeInitShader = ShaderLoader::Instance()->LoadShader(Config::Instance()->project_path + "shader/node_init.cmp.glsl");
	ShaderPtr leafStoreShader = ShaderLoader::Instance()->LoadShader(Config::Instance()->project_path + "shader/leaf_store.cmp.glsl");

	int nodeOffset = 0;
	int allocOffset = 1;
	int dataWidth = 1024;
	int dataHeight = (m_numVoxelFrag + 1023) / dataWidth;
	int groupDimX = dataWidth / 8;
	int groupDimY = (dataHeight + 7) / 8;
	for (int i = 0; i < octreeLevel; ++i)
	{
		//node flag
		nodeFlagShader->Use();
		nodeFlagShader->SetInteger("u_numVoxelFrag", m_numVoxelFrag);
		nodeFlagShader->SetInteger("u_level", i);
		nodeFlagShader->SetInteger("u_voxelDim", voxelSize);
		nodeFlagShader->SetImage("u_voxelPos", m_voxel_pos);
		m_octree_idx->SetImageAccess(GL_READ_WRITE);
		nodeFlagShader->SetImage("u_octreeIdx", m_octree_idx);
		glDispatchCompute(groupDimX, groupDimY, 1);
		m_octree_idx->SyncImage();

		//node tile allocation
		int numThread = allocList[i];
		nodeAllocShader->Use();
		nodeAllocShader->SetInteger("u_num", numThread);
		nodeAllocShader->SetInteger("u_start", nodeOffset);
		nodeAllocShader->SetInteger("u_allocStart", allocOffset);
		nodeAllocShader->SetImage("u_octreeIdx", m_octree_idx);
		int allocGroupDim = (allocList[i] + 63) / 64;
		glDispatchCompute(allocGroupDim, 1, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);

		//Get the number of node tiles to allocate in the next level
		GLuint tileAllocated = allocCounter->GetVal();
		allocCounter->SetVal(0);

		//node tile initialization
		int nodeAllocated = tileAllocated * 8;
		nodeInitShader->Use();
		nodeInitShader->SetInteger("u_num", nodeAllocated);
		nodeInitShader->SetInteger("u_allocStart", allocOffset);
		m_octree_idx->SetImageAccess(GL_WRITE_ONLY);
		nodeInitShader->SetImage("u_octreeIdx", m_octree_idx);
		m_octree_kd->SetImageAccess(GL_WRITE_ONLY);
		nodeInitShader->SetImage("u_octreeKd", m_octree_kd);

		dataWidth = 1024;
		dataHeight = (nodeAllocated + 1023) / dataWidth;
		int initGroupDimX = dataWidth / 8;
		int initGroupDimY = (dataHeight + 7) / 8;
		glDispatchCompute(initGroupDimX, initGroupDimY, 1);
		m_octree_idx->SyncImage();

		//update offsets for next level
		allocList.push_back(nodeAllocated); //titleAllocated * 8 is the number of threads
											  //we want to launch in the next level
		nodeOffset += allocList[i]; //nodeOffset is the starting node in the next level
		allocOffset += nodeAllocated; //allocOffset is the starting address of remaining free space
	}

	//flag nonempty leaf nodes
	nodeFlagShader->Use();
	nodeFlagShader->SetInteger("u_numVoxelFrag", m_numVoxelFrag);
	nodeFlagShader->SetInteger("u_level", octreeLevel);
	nodeFlagShader->SetInteger("u_voxelDim", voxelSize);
	nodeFlagShader->SetImage("u_voxelPos", m_voxel_pos);
	m_octree_idx->SetImageAccess(GL_READ_WRITE);
	nodeFlagShader->SetImage("u_octreeIdx", m_octree_idx);
	glDispatchCompute(groupDimX, groupDimY, 1);
	m_octree_idx->SyncImage();

	//Store surface information ( Color, normal, etc. ) into the octree leaf nodes
	leafStoreShader->Use();
	leafStoreShader->SetInteger("u_numVoxelFrag", m_numVoxelFrag);
	leafStoreShader->SetInteger("u_octreeLevel", octreeLevel);
	leafStoreShader->SetInteger("u_voxelDim", voxelSize);
	m_octree_idx->SetImageAccess(GL_READ_ONLY);
	leafStoreShader->SetImage("u_octreeIdx", m_octree_idx);
	m_octree_kd->SetImageAccess(GL_READ_WRITE);
	leafStoreShader->SetImage("u_octreeKd", m_octree_kd);
	leafStoreShader->SetImage("u_voxelPos", m_voxel_pos);
	leafStoreShader->SetImage("u_voxelKd", m_voxel_kd);
	glDispatchCompute(groupDimX, groupDimY, 1);
	m_octree_kd->SyncImage();
}

void HFRender::RenderOctree()
{
	int voxelSize = Config::Instance()->voxelSize;
	int width = Config::Instance()->width;
	int height = Config::Instance()->height;
	int octreeLevel = Config::Instance()->octreeLevel;

	ParamTable params = {
		{"pointSize", float(height) / float(voxelSize)},
		{"u_voxelDim", voxelSize},
		{"u_level", octreeLevel}
	};

	TextureParamTable image_param = {
		{"u_octreeIdx", m_octree_idx}
	};
	MaterialPtr volumeVisualizationMaterial = Material::CreateMaterial(Config::Instance()->project_path + "shader/Visualization/octree_visualization.vert",
		Config::Instance()->project_path + "shader/Visualization/octree_visualization.frag", "", std::move(params), {}, std::move(image_param));

	VolumePtr volume = std::make_unique<Volume>(glm::vec3(0), 1.0f / voxelSize, voxelSize, voxelSize, voxelSize, volumeVisualizationMaterial);
	m_world.AddEntity("octree_visualization", std::move(volume));

	m_camera.SetPosition(glm::vec3(0, 0, 2));
	m_camera.SetForward(glm::vec3(0, 0, -1));

	glViewport(0, 0, Config::Instance()->width, Config::Instance()->height);

	ViewContext volume_vc;
	volume_vc.SetDepthStates(true, true, GL_LESS);
	m_world.CommitRenderContext(volume_vc, "octree_visualization");

	glEnable(GL_PROGRAM_POINT_SIZE);
	while (!glfwWindowShouldClose(m_window))
	{
		glfwPollEvents();
		ProcessInput();

		m_camera.FillViewContext(volume_vc);
		volume_vc.FlushRenderContext(false);

		glfwSwapBuffers(m_window);
	}
}

int main()
{
	Config::Instance()->project_path = "C:/Users/wanglingye/wly/project/HFRender/";
	Config::Instance()->voxelSize = 128;
	Config::Instance()->octreeLevel = 7;
	HFRender* render = HFRender::Instance();
	render->Init(1024, 780);

	//render->Render();

	render->Voxelize();
	//render->RenderVoxel();
	render->VoxelConeTrace();

	//render->BuildVoxelList();
	//render->BuildSVO();
	//render->RenderOctree();
}

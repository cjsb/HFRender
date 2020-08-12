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

void HFRender::BuildSVO()
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

	m_svo_vct = std::make_shared<SVO_VCT>();
	m_svo_vct->SparseVoxelize(m_voxel_world);
	m_svo_vct->LightUpdate(m_voxel_world);
}


int main()
{
	Config::Instance()->project_path = "C:/Users/wanglingye/wly/project/HFRender/";
	Config::Instance()->voxelSize = 256;
	Config::Instance()->octreeLevel = log2f(256);
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

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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);  // 指定OpenGL次版本号
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

	int voxelTextureSize = Config::Instance()->voxelTextureSize;
	m_texture3D = std::make_shared<Texture3D>(voxelTextureSize, voxelTextureSize, voxelTextureSize, nullptr, GL_FLOAT, false);
	m_texture3D->SetTextureUnit(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindImageTexture(m_texture3D->GetTextureUnit(), m_texture3D->GetId(), 0, GL_TRUE, 0, GL_READ_WRITE, m_texture3D->GetInternalFormat());
	GL_CHECK_ERROR;

	ParamTable params = { {"albedo", glm::vec4(0.f, 1.f, 0.f, 1.f)} };
	TextureParamTable texture_param = {
		{"texture3D", m_texture3D}
	};
	MaterialPtr material = Material::CreateMaterial(Config::Instance()->project_path + "shader/test_voxel.vert",
		Config::Instance()->project_path + "shader/test_voxel.frag", "", std::move(params), std::move(texture_param));

	Transform transform(glm::vec3(0, -0.50f, 0), glm::mat4(1), glm::vec3(1));
	m_world.AddModelEntity(Config::Instance()->project_path + "resource/model/bunny.obj", transform.GetMatrix(), "bunny", material);

	m_camera.SetPosition(glm::vec3(0, 0, 3));
	m_camera.SetForward(glm::vec3(0, 0, -1));

	ViewContext vc;
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

	std::vector<GLfloat> data(4 * voxelTextureSize * voxelTextureSize * voxelTextureSize,1);
	m_texture3D->ReadTextureData(&data[0], GL_FLOAT);
	dump_data(&data[0], 4, voxelTextureSize * voxelTextureSize * voxelTextureSize, "C:\\Users\\wanglingye\\wly\\project\\HFRender\\out\\texture3D.txt");

	return true;
}

void HFRender::Voxelize()
{
	int voxelTextureSize = Config::Instance()->voxelTextureSize;
	glViewport(0, 0, voxelTextureSize, voxelTextureSize);

	Camera camera;
	camera.SetForward(glm::vec3(0, 0, -1));
	camera.SetProjMatrix(glm::ortho(-1, 1, -1, 1, -1, 1));

	ViewContext voxel_vc;
	voxel_vc.SetColorMask(glm::bvec4(false));
	voxel_vc.SetDepthStates(false, false, GL_LESS);
	voxel_vc.SetCullFace(false, GL_BACK);
	voxel_vc.SetBlend(false);
	voxel_vc.SetFramebuffer(m_voxel_FBO);

	camera.FillViewContext(voxel_vc);
	m_world.CommitRenderContext(voxel_vc, "bunny");
	voxel_vc.FlushRenderContext(true);

	m_texture3D->GenerateMipmap();

	std::vector<GLfloat> data(4 * voxelTextureSize * voxelTextureSize * voxelTextureSize);
	m_texture3D->ReadTextureData(&data[0], GL_FLOAT);
	dump_data(&data[0], 4, voxelTextureSize * voxelTextureSize * voxelTextureSize, "C:\\Users\\wanglingye\\wly\\project\\HFRender\\out\\texture3D.txt");
}

void HFRender::InitVoxelize()
{
	int voxelTextureSize = Config::Instance()->voxelTextureSize;
	m_texture3D = std::make_shared<Texture3D>(voxelTextureSize, voxelTextureSize, voxelTextureSize, nullptr, GL_FLOAT, false);
	m_texture3D->SetTextureUnit(0);
	m_voxel_FBO = std::make_shared<Framebuffer>();
	m_voxel_FBO->AttachImage(m_texture3D, GL_WRITE_ONLY);
	m_voxel_FBO->AttachColorBuffer(std::make_unique<RenderSurface>(voxelTextureSize, voxelTextureSize, GL_RGB));
	assert(m_voxel_FBO->CheckStatus());

	ParamTable params = {
		{"material.diffuseColor",glm::vec3(0.95, 1, 0.95)},
		{"material.specularColor",glm::vec3(0.95, 1, 0.95)},
		{"material.diffuseReflectivity",0.0f},
		{"material.specularReflectivity",1.0f},
		{"material.emissivity",0.0f},
		{"material.transparency",0.0f},
		{"pointLights[0].position",glm::vec3(0, 0.975, 0)},
		{"pointLights[0].color",glm::vec3(0.5)},
		{"numberOfLights",1}
	};
	TextureParamTable texture_param = {
		{"texture3D", m_texture3D}
	};
	MaterialPtr material = Material::CreateMaterial(Config::Instance()->project_path + "shader/voxelization.vert",
		Config::Instance()->project_path + "shader/voxelization.frag",
		Config::Instance()->project_path + "shader/voxelization.geom", std::move(params), std::move(texture_param));

	Transform transform(glm::vec3(0, -0.50f, 0), glm::mat4(1), glm::vec3(1));
	m_world.AddModelEntity(Config::Instance()->project_path + "resource/model/bunny.obj", transform.GetMatrix(), "bunny", material);
}

void HFRender::InitRenderVoxel()
{
	int width = Config::Instance()->width;
	int height = Config::Instance()->height;

	Texture2DPtr front_texture = std::make_shared<Texture2D>(width, height, nullptr, GL_FLOAT);
	front_texture->SetTextureUnit(1);
	m_front_FBO = std::make_shared<Framebuffer>();
	m_front_FBO->AttachColorTexture(front_texture);
	m_voxel_FBO->AttachDepthBuffer(std::make_unique<RenderSurface>(width, height, GL_DEPTH24_STENCIL8));
	assert(m_voxel_FBO->CheckStatus());

	Texture2DPtr back_texture = std::make_shared<Texture2D>(width, height, nullptr, GL_FLOAT);
	back_texture->SetTextureUnit(2);
	m_back_FBO = std::make_shared<Framebuffer>();
	m_back_FBO->AttachColorTexture(back_texture);
	m_voxel_FBO->AttachDepthBuffer(std::make_unique<RenderSurface>(width, height, GL_DEPTH24_STENCIL8));
	assert(m_voxel_FBO->CheckStatus());

	MaterialPtr worldPositionMaterial = Material::CreateMaterial(Config::Instance()->project_path + "shader/Visualization/world_position.vert",
		Config::Instance()->project_path + "shader/Visualization/world_position.frag", "", {});
	m_world.AddModelEntity(Config::Instance()->project_path + "resource/model/cube.obj", glm::mat4(1), "world_position", worldPositionMaterial);

	ParamTable params = {
		{"state", 0}
	};
	TextureParamTable texture_param = {
		{"textureBack", back_texture},
		{"textureFront", back_texture},
		{"texture3D", m_texture3D}
	};
	MaterialPtr voxelVisualizationMaterial = Material::CreateMaterial(Config::Instance()->project_path + "shader/Visualization/voxel_visualization.vert",
		Config::Instance()->project_path + "shader/Visualization/voxel_visualization.frag", "", std::move(params), std::move(texture_param));
	m_world.AddModelEntity(Config::Instance()->project_path + "resource/model/quad.obj", glm::mat4(1), "voxel_visualization", voxelVisualizationMaterial);

	m_camera.SetPosition(glm::vec3(0, 0, 2));
	m_camera.SetForward(glm::vec3(0, 0, -1));
}

void HFRender::RenderVoxel()
{
	glViewport(0, 0, Config::Instance()->width, Config::Instance()->height);

	ViewContext cube_vc;
	cube_vc.SetDepthStates(true, true, GL_LESS);
	m_world.CommitRenderContext(cube_vc, "world_position");

	ViewContext screen_vc;
	m_world.CommitRenderContext(screen_vc, "voxel_visualization");

	while (!glfwWindowShouldClose(m_window))
	{
		glfwPollEvents();
		ProcessInput();

		// Render Cube
		m_camera.FillViewContext(cube_vc);
		// Front.
		cube_vc.SetFramebuffer(m_front_FBO);
		cube_vc.SetCullFace(true, GL_BACK);
		cube_vc.FlushRenderContext(false);

		// Back.
		cube_vc.SetFramebuffer(m_back_FBO);
		cube_vc.SetCullFace(true, GL_FRONT);
		cube_vc.FlushRenderContext(false);

		// Render 3D texture to screen.
		m_camera.FillViewContext(screen_vc);
		screen_vc.FlushRenderContext(false);

		glfwSwapBuffers(m_window);
	}
}

int main()
{
	Config::Instance()->project_path = "C:/Users/wanglingye/wly/project/HFRender/";
	HFRender* render = HFRender::Instance();
	render->Init(1024, 780);

	render->Render();

	//render->InitVoxelize();
	//render->Voxelize();

	//render->InitRenderVoxel();
	//render->RenderVoxel();
}

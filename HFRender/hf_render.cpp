#include "hf_render.h"
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>

const char* glsl_version = "#version 330";
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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);  // 指定OpenGL主版本号
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);  // 指定OpenGL次版本号
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
	ImGui::Combo("Render mode", reinterpret_cast<int*>(&m_config.render_mode), items, IM_ARRAYSIZE(items));

	ImGui::End();
}

bool HFRender::Render()
{

	while (!glfwWindowShouldClose(m_window))
	{
		glfwPollEvents();
		ProcessInput();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// render your GUI
		RenderImGui();

		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(m_window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(m_window);
	}

	std::cout << static_cast<int>(m_config.render_mode) << std::endl;

	return true;
}

int main()
{
	HFRender render;
	render.Init(1024, 780);

	render.Render();
}

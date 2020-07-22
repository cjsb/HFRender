#pragma once
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "camera.h"

const float DEFAULT_CAMERA_MOVE_SPEED = 500.0f;
const float DEFAULT_MOUSE_SENSITIVITY = 0.001f;

class HFRender
{
public:
	HFRender() {}
	~HFRender() {}

	bool Init(int width, int height);
	void Destroy();

	bool Render();
	static HFRender* Instance()
	{
		return s_inst;
	}

private:
	bool InitGlfw();
	bool InitGlad();
	void ProcessInput();
	void ProcessMouseMovement(float xoffset, float yoffset);
	void ProcessMouseScroll(float yoffset);
	static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
	static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
	static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

	float m_last_time = 0.f;
	float m_last_mouse_x = std::numeric_limits<float>::quiet_NaN();
	float m_last_mouse_y = std::numeric_limits<float>::quiet_NaN();
	float m_camera_move_speed = DEFAULT_CAMERA_MOVE_SPEED;
	float m_mouse_sensitivity = DEFAULT_MOUSE_SENSITIVITY;

	GLFWwindow* m_window;
	Camera m_camera;

	static HFRender* s_inst;
};

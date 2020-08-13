#pragma once
#include <iostream>
#include "opengl_common.h"
#include "camera.h"
#include "world.h"
#include "config.h"
#include "svo_vct.h"

const float DEFAULT_CAMERA_MOVE_SPEED = 1.f;
const float DEFAULT_MOUSE_SENSITIVITY = 0.001f;

class HFRender
{
public:
	~HFRender() {}

	static HFRender* Instance()
	{
		return s_inst;
	}

	bool Init(int width, int height);
	void Destroy();

	bool Render();
	void RenderWorld();

	void Voxelize();
	void RenderVoxel();
	void VoxelConeTrace();

	void LoadWorld();
	void BuildSVO();
	void RenderVoxelList();
	void RenderOctree();

private:
	HFRender() {}
	bool InitGlfw();
	bool InitGlad();
	void RenderImGui();

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
	World m_world;
	World m_voxel_world;

	Texture3DPtr m_texture3D;

	SVO_VCTPtr m_svo_vct;

	static HFRender* s_inst;
};

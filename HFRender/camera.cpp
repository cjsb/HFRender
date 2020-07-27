#include "camera.h"

Camera::Camera()
{
	LookAt(glm::vec3(1.f, 0.f, 0.f));
	m_proj_mat = glm::perspective(glm::radians(45.f), 1024.f / 768.f, 0.1f, 10.0f);
}

void Camera::SetPosition(const glm::vec3& pos)
{
	m_position = pos;
	UpdateViewMatrix();
}

void Camera::SetForward(const glm::vec3& forward)
{
	m_forward = glm::normalize(forward);
	UpdateViewMatrix();
}

void Camera::LookAt(const glm::vec3& target)
{
	m_forward = glm::normalize(target - m_position);
	if (m_forward.length() == 0.f)
	{
		m_forward = glm::vec3(1.f, 0.f, 0.f);
	}
	UpdateViewMatrix();
}

void Camera::FillViewContext(ViewContext& vc)
{
	vc.SetViewMat(m_view_mat);
	vc.SetProjMat(m_proj_mat);
}

void Camera::UpdateViewMatrix()
{
	glm::vec3 target = m_position + m_forward;
	if (m_forward.x == 0.f && m_forward.z == 0.f)
	{
		m_right = glm::vec3(1.0f, 0.0f, 0.0f);
	}
	else
	{
		m_right = glm::normalize(glm::cross(m_forward, glm::vec3(0.0f, 1.0f, 0.0f)));
	}
	m_up = glm::normalize(glm::cross(m_right, m_forward));
	m_view_mat = glm::lookAt(m_position, target, m_up);
	m_inv_view_mat = glm::inverse(m_view_mat);
}
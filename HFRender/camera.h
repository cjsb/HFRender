#pragma once
#include "render_context.h"

class Camera
{
public:
	Camera();

	const glm::mat4& GetViewMatrix() const { return m_view_mat; }
	const glm::mat4& GetInvViewMatrix() const { return m_inv_view_mat; }
	const glm::mat4& GetProjMatrix() const { return m_proj_mat; }
	void SetProjMatrix(const glm::mat4& proj_mat) { m_proj_mat = proj_mat; }
	const glm::vec3& GetPosition() const { return m_position; }
	void SetPosition(const glm::vec3& pos);
	const glm::vec3& GetForward() const { return m_forward; }
	void SetForward(const glm::vec3& forward);
	const glm::vec3& GetRight() const { return m_right; }
	const glm::vec3& GetUp() const { return m_up; }
	void LookAt(const glm::vec3& target);
	void FillViewContext(ViewContext& vc);
private:
	void UpdateViewMatrix();

	glm::mat4 m_view_mat = glm::mat4(1.f);
	glm::mat4 m_inv_view_mat = glm::mat4(1.f);
	glm::mat4 m_proj_mat;

	glm::vec3 m_forward;
	glm::vec3 m_up;
	glm::vec3 m_right;
	glm::vec3 m_position = glm::vec3(0.f);
};
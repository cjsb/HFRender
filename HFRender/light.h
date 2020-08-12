#pragma once
#include "opengl_common.h"

class DirectionLight
{
public:
	DirectionLight(const glm::vec3& direction, const glm::vec3& color);
	const glm::vec3& GetDirection() const { return m_direction; }
	const glm::vec3& GetColor() const { return m_color; }
	const glm::mat4& GetLightViewMatrix() const { return m_view_mat; }
	const glm::mat4& GetLightProjMatrix() const { return m_proj_mat; }
private:
	glm::vec3 m_direction;
	glm::vec3 m_color;
	glm::mat4 m_view_mat;
	glm::mat4 m_proj_mat;
};
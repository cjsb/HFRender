#include "light.h"

DirectionLight::DirectionLight(const glm::vec3& direction, const glm::vec3& color) :m_direction(direction), m_color(color)
{
	glm::vec3 right;
	if (direction.x == 0.f && direction.z == 0.f)
	{
		right = glm::vec3(1.0f, 0.0f, 0.0f);
	}
	else
	{
		right = glm::normalize(glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f)));
	}

	glm::vec3 up = glm::normalize(glm::cross(right, direction));
	m_view_mat = glm::lookAt(glm::vec3(0), direction, up);

	float sqrt_2 = sqrtf(2);
	m_proj_mat = glm::ortho(-sqrt_2, sqrt_2, -sqrt_2, sqrt_2, -sqrt_2, sqrt_2);
}
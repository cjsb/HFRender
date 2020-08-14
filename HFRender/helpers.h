#pragma once
#include <string>
#include <vector>
#include "opengl_common.h"

std::vector<std::string> Split(const std::string& s, const std::string& t);

void dump_data(float* data, uint32_t stride, uint32_t count, const std::string& path);

void dump_3D_data(float* data, uint32_t stride, uint32_t width, uint32_t height, uint32_t depth, const std::string& path);

void dump_buffer_data(uint32_t* data, uint32_t num, const std::string& path);

void dump_node_idx(uint32_t* data, uint32_t num, const std::string& path);

int convVec4ToRGBA8(const glm::vec4& val);

class Transform
{
public:
	Transform(const glm::vec3& pos, const glm::mat4& rot, const glm::vec3& scale) :
		m_position(pos), m_rotation(rot), m_scale(scale)
	{
		m_trans_mat = glm::mat4(1.0f);
		m_trans_mat = glm::translate(m_trans_mat, pos);
		m_trans_mat = m_trans_mat * rot;
		m_trans_mat = glm::scale(m_trans_mat, scale);
	}

	Transform(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale) :
		m_position(pos), m_scale(scale)
	{
		m_rotation = glm::mat4_cast(glm::quat(rot));

		m_trans_mat = glm::mat4(1.0f);
		m_trans_mat = glm::translate(m_trans_mat, pos);
		m_trans_mat = m_trans_mat * m_rotation;
		m_trans_mat = glm::scale(m_trans_mat, scale);
	}

	void addTranslation(const glm::vec3& pos)
	{
		m_position += pos;
		m_trans_mat = glm::mat4(1.0f);
		m_trans_mat = glm::translate(m_trans_mat, m_position);
		m_trans_mat = m_trans_mat * m_rotation;
		m_trans_mat = glm::scale(m_trans_mat, m_scale);
	}

	const glm::mat4& GetMatrix() const { return m_trans_mat; }
private:
	glm::vec3 m_position;
	glm::vec3 m_scale;
	glm::mat4 m_rotation;
	glm::mat4 m_trans_mat;
};
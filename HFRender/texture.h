#pragma once
#include <cstdint>
#include <memory>
#include "opengl_common.h"

class Texture
{
public:
	Texture(uint32_t width, uint32_t height, void* data, GLuint data_type = GL_UNSIGNED_BYTE,
		GLuint internal_format = GL_RGB, GLuint image_format = GL_RGB,
		GLuint warp_s = GL_REPEAT, GLuint warp_t = GL_REPEAT,
		GLuint filter_min = GL_LINEAR, GLuint filter_max = GL_LINEAR);

	~Texture();

	void Use() const 
	{ 
		glBindTexture(GL_TEXTURE_2D, m_id); 
		GL_CHECK_ERROR;
	}

	GLuint GetId() const { return m_id; }

private:
	GLuint m_id;
	uint32_t m_width;
	uint32_t m_height;
};

typedef std::shared_ptr<Texture> TexturePtr;
#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include "opengl_common.h"

class ITexture
{
public:
	virtual void Activate() = 0;
	virtual GLuint GetId() = 0;
	virtual void SetTextureUnit(GLuint textureUnit) = 0;
	virtual GLuint GetTextureUnit() = 0;
	virtual GLenum GetInternalFormat() = 0;
	virtual void GenerateMipmap() = 0;
};

typedef std::shared_ptr<ITexture> ITexturePtr;

class Texture2D:public ITexture
{
public:
	Texture2D(uint32_t width, uint32_t height, void* data, GLuint data_type = GL_UNSIGNED_BYTE, bool generateMipmaps = false);

	~Texture2D();

	virtual void Activate() override
	{
		glActiveTexture(GL_TEXTURE0 + m_textureUnit);
		glBindTexture(GL_TEXTURE_2D, m_id);
		GL_CHECK_ERROR;
	}

	virtual GLuint GetId() override { return m_id; }
	virtual void SetTextureUnit(GLuint textureUnit) override { m_textureUnit = textureUnit; }
	virtual GLuint GetTextureUnit() override { return m_textureUnit; }
	virtual GLenum GetInternalFormat()override { return m_internal_format; }
	virtual void GenerateMipmap()override;
private:
	GLuint m_id;
	uint32_t m_width;
	uint32_t m_height;
	GLuint m_textureUnit = 0;
	GLenum m_internal_format;
};

typedef std::shared_ptr<Texture2D> Texture2DPtr;

class Texture3D :public ITexture
{
public:
	Texture3D(uint32_t width, uint32_t height, uint32_t depth, void* data, GLenum data_type = GL_UNSIGNED_BYTE, bool generateMipmaps = false);

	~Texture3D();

	void Activate() override
	{
		glActiveTexture(GL_TEXTURE0 + m_textureUnit);
		glBindTexture(GL_TEXTURE_3D, m_id);
		GL_CHECK_ERROR;
	}

	virtual GLuint GetId() override { return m_id; }
	virtual void SetTextureUnit(GLuint textureUnit) override { m_textureUnit = textureUnit; }
	virtual GLuint GetTextureUnit() override { return m_textureUnit; }
	virtual GLenum GetInternalFormat()override { return m_internal_format; }
	virtual void GenerateMipmap()override;
	void ReadTextureData(void* data, GLenum data_type);
private:
	GLuint m_id;
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_depth;
	GLuint m_textureUnit = 0;
	GLenum m_internal_format;
};

typedef std::shared_ptr<Texture3D> Texture3DPtr;
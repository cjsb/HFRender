#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include "opengl_common.h"

class ITexture
{
public:
	virtual void ActivateTexture() = 0;
	virtual GLuint GetId() = 0;
	virtual void SetUnit(GLuint unit) = 0;
	virtual GLuint GetUnit() = 0;
	virtual GLenum GetInternalFormat() = 0;
	virtual void GenerateMipmap() = 0;
	virtual void BindImage() = 0;
	virtual void SetImageAccess(GLenum access) = 0;
	virtual void SyncImage()
	{
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		GL_CHECK_ERROR;
	}
};

typedef std::shared_ptr<ITexture> ITexturePtr;

class Texture2D:public ITexture
{
public:
	Texture2D(uint32_t width, uint32_t height, void* data, GLuint data_type = GL_UNSIGNED_BYTE, bool generateMipmaps = false);
	Texture2D(uint32_t width, uint32_t height, GLenum internal_format, GLenum format, GLenum warp,
		void* data, GLuint data_type = GL_UNSIGNED_BYTE, bool generateMipmaps = false);

	~Texture2D();

	virtual void ActivateTexture() override
	{
		glActiveTexture(GL_TEXTURE0 + m_unit);
		glBindTexture(GL_TEXTURE_2D, m_id);
		GL_CHECK_ERROR;
	}

	virtual void BindImage() override
	{
		glBindImageTexture(m_unit, m_id, 0, GL_FALSE, 0, m_access, m_internal_format);
		GL_CHECK_ERROR;
	}

	virtual void SetImageAccess(GLenum access) override { m_access = access; }
	virtual GLuint GetId() override { return m_id; }
	virtual void SetUnit(GLuint unit) override { m_unit = unit; }
	virtual GLuint GetUnit() override { return m_unit; }
	virtual GLenum GetInternalFormat()override { return m_internal_format; }
	virtual void GenerateMipmap()override;
private:
	GLuint m_id;
	uint32_t m_width;
	uint32_t m_height;
	GLuint m_unit = 0;
	GLenum m_internal_format;
	GLenum m_access = GL_READ_WRITE;
};

typedef std::shared_ptr<Texture2D> Texture2DPtr;

class Texture3D :public ITexture
{
public:
	Texture3D(uint32_t width, uint32_t height, uint32_t depth, GLenum internal_format, GLenum format, bool generateMipmaps = false,
		void* data=nullptr, GLenum data_type = GL_UNSIGNED_BYTE);

	~Texture3D();

	virtual void ActivateTexture() override
	{
		glActiveTexture(GL_TEXTURE0 + m_unit);
		glBindTexture(GL_TEXTURE_3D, m_id);
		GL_CHECK_ERROR;
	}

	virtual void BindImage() override
	{
		glBindImageTexture(m_unit, m_id, 0, GL_TRUE, 0, m_access, m_internal_format);
		GL_CHECK_ERROR;
	}

	virtual void SetImageAccess(GLenum access) override { m_access = access; }
	virtual GLuint GetId() override { return m_id; }
	virtual void SetUnit(GLuint unit) override { m_unit = unit; }
	virtual GLuint GetUnit() override { return m_unit; }
	virtual GLenum GetInternalFormat()override { return m_internal_format; }
	virtual void GenerateMipmap()override;
	void ReadTextureData(void* data, GLenum data_type);
private:
	GLuint m_id;
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_depth;
	GLuint m_unit = 0;
	GLenum m_internal_format;
	GLenum m_format;
	GLenum m_access = GL_READ_WRITE;
};

typedef std::shared_ptr<Texture3D> Texture3DPtr;


class TextureBuffer :public ITexture
{
public:
	TextureBuffer(const void* data, size_t len, GLenum format);
	~TextureBuffer();

	virtual void ActivateTexture() override
	{
		glActiveTexture(GL_TEXTURE0 + m_unit);
		glBindTexture(GL_TEXTURE_BUFFER, m_texture_id);
		GL_CHECK_ERROR;
	}

	virtual void BindImage() override
	{
		glBindImageTexture(m_unit, m_texture_id, 0, GL_FALSE, 0, m_access, m_internal_format);
		GL_CHECK_ERROR;
	}

	virtual void SetImageAccess(GLenum access) override { m_access = access; }
	virtual GLuint GetId() override { return m_texture_id; }
	virtual void SetUnit(GLuint unit) override { m_unit = unit; }
	virtual GLuint GetUnit() override { return m_unit; }
	virtual GLenum GetInternalFormat() override { return m_internal_format; }
	virtual void GenerateMipmap()override {}
	void SetInternalFormat(GLenum internal_format) { m_internal_format = internal_format; }
	void ReadTextureData(void* data, size_t len);
protected:
	GLuint m_texture_id;
	GLuint m_buffer_id;
	GLenum m_internal_format;
	GLenum m_format;
	GLuint m_unit = 0;
	GLenum m_access = GL_READ_WRITE;
};

typedef std::shared_ptr<TextureBuffer> TextureBufferPtr;
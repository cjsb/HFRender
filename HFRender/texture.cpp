#include "texture.h"
#include <vector>

Texture2D::Texture2D(uint32_t width, uint32_t height, void* data, GLuint data_type, bool generateMipmaps):m_width(width), m_height(height)
{
    glGenTextures(1, &m_id);
    GL_CHECK_ERROR;

    m_internal_format = GL_RGB;
    // Create Texture
    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexImage2D(GL_TEXTURE_2D, 0, m_internal_format, width, height, 0, GL_RGB, data_type, data);
    GL_CHECK_ERROR;

    // Set Texture wrap and filter modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GL_CHECK_ERROR;

    if (generateMipmaps)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
        GL_CHECK_ERROR;
    }

    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture2D::Texture2D(uint32_t width, uint32_t height, GLenum internal_format, GLenum format, 
    GLenum warp, void* data, GLuint data_type, bool generateMipmaps) 
    :m_width(width), m_height(height), m_internal_format(internal_format)
{
    glGenTextures(1, &m_id);
    GL_CHECK_ERROR;

    // Create Texture
    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, data_type, data);
    GL_CHECK_ERROR;

    // Set Texture wrap and filter modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, warp);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, warp);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GL_CHECK_ERROR;

    if (generateMipmaps)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
        GL_CHECK_ERROR;
    }

    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture2D::~Texture2D()
{
    if (m_id != 0)
    {
        glDeleteTextures(1, &m_id);
        GL_CHECK_ERROR;
        m_id = 0;
    }
}


void Texture2D::GenerateMipmap()
{
    glBindTexture(GL_TEXTURE_2D, m_id);
    glGenerateMipmap(GL_TEXTURE_2D);
    GL_CHECK_ERROR;
}

Texture3D::Texture3D(uint32_t width, uint32_t height, uint32_t depth, GLenum internal_format, GLenum format, bool generateMipmaps,
    void* data, GLenum data_type) :m_width(width), m_height(height), m_depth(depth), m_internal_format(internal_format), m_format(format)
{
    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_3D, m_id);
    GL_CHECK_ERROR;

    // Parameter options.
    float borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, borderColor);

    GLenum wrap = GL_CLAMP_TO_BORDER;
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrap);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrap);

    //这里要根据是否生产MipMap来改变GL_TEXTURE_MIN_FILTER的值
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage3D(GL_TEXTURE_3D, 0, internal_format, width, height, depth, 0, format, data_type, data);
    GL_CHECK_ERROR;

    if (generateMipmaps)
    {
        glGenerateMipmap(GL_TEXTURE_3D);
        GL_CHECK_ERROR;
    }
    glBindTexture(GL_TEXTURE_3D, 0);
}

Texture3D::~Texture3D()
{
    if (m_id != 0)
    {
        glDeleteTextures(1, &m_id);
        GL_CHECK_ERROR;
        m_id = 0;
    }
}

void Texture3D::GenerateMipmap()
{
    ActivateTexture();
    glGenerateMipmap(GL_TEXTURE_3D);
    GL_CHECK_ERROR;
}

void Texture3D::ReadTextureData(void* data, GLenum data_type)
{
    ActivateTexture();
    glGetTexImage(GL_TEXTURE_3D, 0, m_format, data_type, data);
    GL_CHECK_ERROR;
}

TextureBuffer::TextureBuffer(const void* data, size_t len, GLenum format) :m_format(format), m_internal_format(format)
{
    glGenBuffers(1, &m_buffer_id);
    glBindBuffer(GL_TEXTURE_BUFFER, m_buffer_id);
    glBufferData(GL_TEXTURE_BUFFER, len, data, GL_STATIC_DRAW);
    GL_CHECK_ERROR;

    glGenTextures(1, &m_texture_id);
    glBindTexture(GL_TEXTURE_BUFFER, m_texture_id);
    glTexBuffer(GL_TEXTURE_BUFFER, format, m_buffer_id);
    GL_CHECK_ERROR;
}

TextureBuffer::~TextureBuffer()
{
    if (m_buffer_id)
    {
        glDeleteBuffers(1, &m_buffer_id);
        m_buffer_id = 0;
    }

    if (m_texture_id)
    {
        glDeleteTextures(1, &m_texture_id);
        m_texture_id = 0;
    }
}

void TextureBuffer::ReadTextureData(void* data, size_t len)
{
    glBindBuffer(GL_TEXTURE_BUFFER, m_buffer_id);
    glGetBufferSubData(GL_TEXTURE_BUFFER, 0, len, data);
    GL_CHECK_ERROR;
}
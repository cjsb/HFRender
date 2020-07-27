#include "texture.h"

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

Texture2D::~Texture2D()
{
    if (m_id != 0)
    {
        glDeleteTextures(1, &m_id);
        GL_CHECK_ERROR;
        m_id = 0;
    }
}

Texture3D::Texture3D(uint32_t width, uint32_t height, uint32_t depth, void* data, GLuint data_type, bool generateMipmaps) :m_width(width), m_height(height), m_depth(depth)
{
    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_3D, m_id);
    GL_CHECK_ERROR;

    // Parameter options.
    const auto wrap = GL_CLAMP_TO_BORDER;
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrap);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrap);

    const auto filter = GL_LINEAR_MIPMAP_LINEAR;
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Upload texture buffer.
    const int levels = 7;
    m_internal_format = GL_RGBA8;
    glTexStorage3D(GL_TEXTURE_3D, levels, m_internal_format, width, height, depth);
    glTexImage3D(GL_TEXTURE_3D, 0, m_internal_format, width, height, depth, 0, GL_RGBA, data_type, data);
    if (generateMipmaps)
    {
        glGenerateMipmap(GL_TEXTURE_3D);
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
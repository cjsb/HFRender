#include "texture.h"

Texture::Texture(uint32_t width, uint32_t height, void* data, GLuint data_type,
    GLuint internal_format, GLuint image_format, 
    GLuint warp_s, GLuint warp_t, 
    GLuint filter_min, GLuint filter_max)
{
    glGenTextures(1, &m_id);
    GL_CHECK_ERROR;

    // Create Texture
    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, image_format, data_type, data);
    GL_CHECK_ERROR;

    // Set Texture wrap and filter modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, warp_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, warp_t);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_min);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_max);
    GL_CHECK_ERROR;

    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture()
{
    if (m_id != 0)
    {
        glDeleteTextures(1, &m_id);
        GL_CHECK_ERROR;
        m_id = 0;
    }
}
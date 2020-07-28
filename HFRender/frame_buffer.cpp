#include "frame_buffer.h"

Framebuffer::Framebuffer(bool is_default)
{
	if (!is_default)
	{
		glGenFramebuffers(1, &m_id);
		GL_CHECK_ERROR;
	}
}

Framebuffer::~Framebuffer()
{
	if (m_id != 0)
	{
		glDeleteFramebuffers(1, &m_id);
		GL_CHECK_ERROR;
		m_id = 0;
	}
}

void Framebuffer::AttachColorBuffer(RenderSurfacePtr&& color_surface)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, m_color_attachment, GL_RENDERBUFFER, color_surface->GetId());
	GL_CHECK_ERROR;
	m_renderbuffers.emplace_back(std::move(color_surface));
	m_color_attachment++;
}

void Framebuffer::AttachDepthBuffer(RenderSurfacePtr&& depth_surface)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_surface->GetId());
	GL_CHECK_ERROR;
	m_renderbuffers.emplace_back(std::move(depth_surface));
}

void Framebuffer::AttachColorTexture(const Texture2DPtr& color_texture)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);
	glFramebufferTexture2D(GL_FRAMEBUFFER, m_color_attachment, GL_TEXTURE_2D, color_texture->GetId(), 0);
	GL_CHECK_ERROR;
	m_texture2Ds.emplace_back(color_texture);
	m_color_attachment++;
}

void Framebuffer::AttachDepthTexture(const Texture2DPtr& depth_texture)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth_texture->GetId(), 0);
	GL_CHECK_ERROR;
	m_depth_texture = depth_texture;
}

void Framebuffer::AttachImage(const ITexturePtr& texture, GLenum access)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);
	glBindImageTexture(texture->GetTextureUnit(), texture->GetId(), 0, GL_TRUE, 0, access, texture->GetInternalFormat());
	GL_CHECK_ERROR;
	m_images.emplace_back(texture);
}

bool Framebuffer::CheckStatus()
{
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	return status == GL_FRAMEBUFFER_COMPLETE;
}

void Framebuffer::Use()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);
	GL_CHECK_ERROR;
}
#include "framebuffer.h"

Framebuffer::Framebuffer()
{
	glGenFramebuffers(1, &m_id);
	GL_CHECK_ERROR;
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
	GL_CHECK_ERROR;
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color_surface->GetId());
	GL_CHECK_ERROR;
	m_renderbuffers.emplace_back(std::move(color_surface));
}

void Framebuffer::AttachDepthBuffer(RenderSurfacePtr&& depth_surface)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);
	GL_CHECK_ERROR;
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_surface->GetId());
	GL_CHECK_ERROR;
	m_renderbuffers.emplace_back(std::move(depth_surface));
}

void Framebuffer::AttachColorTexture(TexturePtr&& color_texture)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);
	GL_CHECK_ERROR;
	glFramebufferTexture2D(GL_FRAMEBUFFER, m_color_attachment, GL_TEXTURE_2D, color_texture->GetId(), 0);
	GL_CHECK_ERROR;
	m_textures.emplace_back(std::move(color_texture));
	m_color_attachment++;
}

void Framebuffer::AttachDepthTexture(TexturePtr&& depth_texture)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);
	GL_CHECK_ERROR;
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth_texture->GetId(), 0);
	GL_CHECK_ERROR;
	m_textures.emplace_back(std::move(depth_texture));
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
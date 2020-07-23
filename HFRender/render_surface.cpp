#include "render_surface.h"

RenderSurface::RenderSurface(uint32_t width, uint32_t height, GLenum sized_internal_format) :
	m_width(width),
	m_height(height),
	m_sized_internal_format(sized_internal_format)
{
	glGenRenderbuffers(1, &m_id);
	GL_CHECK_ERROR;

	glBindRenderbuffer(GL_RENDERBUFFER, m_id);
	GL_CHECK_ERROR;

	glRenderbufferStorage(GL_RENDERBUFFER, sized_internal_format, width, height);
	GL_CHECK_ERROR;
}

RenderSurface::~RenderSurface()
{
	if (m_id != 0)
	{
		glDeleteRenderbuffers(1, &m_id);
		GL_CHECK_ERROR;
		m_id = 0;
	}
}

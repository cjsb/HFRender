#pragma once
#include <cstdint>
#include <memory>
#include "opengl_common.h"

class RenderSurface
{
public:
	RenderSurface(uint32_t width, uint32_t height, GLenum sized_internal_format);
	~RenderSurface();
	GLuint GetId() const { return m_id; }
protected:
	GLuint m_id = 0;
	uint32_t m_width;
	uint32_t m_height;
	GLenum m_sized_internal_format;
};

typedef std::unique_ptr<RenderSurface> RenderSurfacePtr;

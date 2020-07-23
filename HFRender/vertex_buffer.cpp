#include "vertex_buffer.h"

VertexBuffer::VertexBuffer(BufferType type, const void* data, size_t len)
{
	switch (type)
	{
	case BufferType::VERTEX:
		m_target = GL_ARRAY_BUFFER;
		break;
	case BufferType::INDEX:
		m_target = GL_ELEMENT_ARRAY_BUFFER;
		break;
	default:
		assert(0);
	}

	glGenBuffers(1, &m_id);
	GL_CHECK_ERROR;

	glBindBuffer(m_target, m_id);
	GL_CHECK_ERROR;

	glBufferData(m_target, len, data, GL_STATIC_DRAW);
	GL_CHECK_ERROR;
}

VertexBuffer::~VertexBuffer()
{
	if (m_id)
	{
		glDeleteBuffers(1, &m_id);
		m_id = 0;
	}
}

void VertexBuffer::Bind()
{
	glBindBuffer(m_target, m_id);
	GL_CHECK_ERROR;
}
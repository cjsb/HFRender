#pragma once
#include <memory>
#include "opengl_common.h"

class VertexBuffer
{
public:
	enum class BufferType : uint8_t
	{
		VERTEX = 0,
		INDEX
	};

	VertexBuffer(BufferType type, const void* data, size_t len);
	~VertexBuffer();
	void Bind();
protected:
	GLuint m_id;
	GLenum m_target;
};

typedef std::shared_ptr<VertexBuffer> VertexBufferPtr;
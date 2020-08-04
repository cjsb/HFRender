#pragma once
#include <memory>
#include "opengl_common.h"

class AutomicBuffer
{
public:
	AutomicBuffer(GLuint initVal=0);
	~AutomicBuffer();
	void BindBase(GLuint bind);
	void Bind();
	GLuint GetVal();
	void SetVal(GLuint val);
	void Sync();
protected:
	GLuint m_id;
	GLuint m_val;
};

typedef std::shared_ptr<AutomicBuffer> AutomicBufferPtr;
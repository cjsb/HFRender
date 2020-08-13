#include "automic_buffer.h"

AutomicBuffer::AutomicBuffer(GLuint initVal)
{
    glGenBuffers(1, &m_id);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_id);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &initVal, GL_STATIC_DRAW);
    GL_CHECK_ERROR;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
}

AutomicBuffer::~AutomicBuffer()
{
    if (m_id)
    {
        glDeleteBuffers(1, &m_id);
        m_id = 0;
    }
}

void AutomicBuffer::BindBase(GLuint bind)
{
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, bind, m_id);
    GL_CHECK_ERROR;
}

void AutomicBuffer::Bind()
{
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_id);
    GL_CHECK_ERROR;
}

GLuint AutomicBuffer::GetVal()
{
    GLuint val;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_id);
    glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &val);
    GL_CHECK_ERROR;
    return val;
}

void AutomicBuffer::SetVal(GLuint val)
{
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_id);
    glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &val);
    GL_CHECK_ERROR;
}

void AutomicBuffer::Sync()
{
    glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT);
    GL_CHECK_ERROR;
}
#include "render_context.h"

void RenderContext::Render(const ViewContext& view_context)
{
	if (m_vao == 0 || !m_material)
	{
		return;
	}

	const ShaderPtr& shader = m_material->GetShader();
	shader->Use();
	shader->SetMatrix4("view", view_context.GetViewMat());
	shader->SetMatrix4("projection", view_context.GetProjMat());
	shader->SetMatrix4("model", m_transform);
	m_material->Apply();
	glBindVertexArray(m_vao);
	GL_CHECK_ERROR;

	glDrawElements(GL_TRIANGLES, m_prim_count * 3, GL_UNSIGNED_SHORT, m_index_offset);
	GL_CHECK_ERROR;

	glBindVertexArray(0);
}

void ViewContext::SetDepthStates(bool depth_test_enabled, bool depth_mask, GLenum depth_func)
{
	m_depth_test_enabled = depth_test_enabled;
	m_depth_mask = depth_mask;
	m_depth_func = depth_func;
}

void ViewContext::SetStencilStates(bool stencil_enable, GLenum stencil_func, GLenum stencil_fail_op,
	GLenum depth_fail_op, GLenum pass_op, GLuint stencil_read_mask, GLuint stencil_write_mask, GLint stencil_ref)
{
	m_stencil_enable = stencil_enable;
	m_stencil_func = stencil_func;
	m_stencil_fail_op = stencil_fail_op;
	m_depth_fail_op = depth_fail_op;
	m_pass_op = pass_op;
	m_stencil_read_mask = stencil_read_mask;
	m_stencil_write_mask = stencil_write_mask;
	m_stencil_ref = stencil_ref;
}

void ViewContext::ApplyRenderStates()
{
	glPolygonMode(GL_FRONT_AND_BACK, m_polygon_mode);

	glColorMask(
		m_color_mask.r ? GL_TRUE : GL_FALSE,
		m_color_mask.g ? GL_TRUE : GL_FALSE,
		m_color_mask.b ? GL_TRUE : GL_FALSE,
		m_color_mask.a ? GL_TRUE : GL_FALSE);

	if (m_depth_test_enabled)
	{
		glEnable(GL_DEPTH_TEST);

		glDepthMask(m_depth_mask ? GL_TRUE : GL_FALSE);

		glDepthFunc(m_depth_func);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}

	if (m_stencil_enable)
	{
		glEnable(GL_STENCIL_TEST);
	}
	else
	{
		glDisable(GL_STENCIL_TEST);
	}

	glStencilFunc(m_stencil_func, m_stencil_ref, m_stencil_read_mask);

	glStencilOp(m_stencil_fail_op, m_depth_fail_op, m_pass_op);

	glStencilMask(m_stencil_write_mask);

	if (m_cull_face_enabled)
	{
		glEnable(GL_CULL_FACE);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}

	if (m_blend_enabled)
	{
		glEnable(GL_BLEND);
	}
	else
	{
		glDisable(GL_BLEND);
	}
}

void ViewContext::ApplyFrameBuffer()
{
	if (m_fb)
	{
		m_fb->Use();

		const glm::vec4& clear_color = m_fb->GetClearColor();
		glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
		GL_CHECK_ERROR;

		glClearDepthf(m_fb->GetClearDepth());
		GL_CHECK_ERROR;

		glClearStencil(m_fb->GetClearStencil());
		GL_CHECK_ERROR;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		GL_CHECK_ERROR;
	}
	else
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GL_CHECK_ERROR;

		glClearColor(0.f, 0.f, 0.f, 1.f);
		GL_CHECK_ERROR;

		glClearDepthf(1.f);
		GL_CHECK_ERROR;

		glClearStencil(0);
		GL_CHECK_ERROR;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		GL_CHECK_ERROR;
	}
}
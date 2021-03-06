#pragma once
#include <unordered_set>
#include "shader.h"
#include "material.h"
#include "frame_buffer.h"

class ViewContext;

class RenderContext
{
public:
	void Render(const ViewContext& view_context);

	void SetVao(GLuint vao)
	{
		assert(vao != 0);
		m_vao = vao;
	}
	void SetTransform(const glm::mat4& transform) { m_transform = transform; }
	void SetMaterial(const MaterialPtr& material) { m_material = material; }
	void SetPointCount(uint32_t point_count) { m_point_count = point_count; }
	void SetPrimCount(uint32_t prim_count) { m_prim_count = prim_count; }
	void SetIndexOffset(void* offset) { m_index_offset = offset; }
	void SetRenderMode(GLenum render_mode) { m_render_mode = render_mode; }
protected:
	GLuint m_vao = 0;
	glm::mat4 m_transform = glm::mat4(1.f);
	MaterialPtr m_material;
	GLuint m_point_count;
	GLuint m_prim_count;
	GLvoid* m_index_offset = 0;
	GLenum m_render_mode = GL_TRIANGLES;
};

class ViewContext
{
public:
	glm::mat4 GetViewMat() const { return m_view; }
	void SetViewMat(const glm::mat4& view) { m_view = view; }
	glm::mat4 GetProjMat() const { return m_proj; }
	void SetProjMat(const glm::mat4& proj) { m_proj = proj; }
	glm::vec3 GetCameraPosition()const { return m_camera_position; }
	void SetCameraPosition(const glm::vec3& cameraPosition) { m_camera_position = cameraPosition; }
	void ClearRenderContext() { m_rcs.clear(); }
	void SetPolygonMode(GLenum polygon_mode) { m_polygon_mode = polygon_mode; }
	void SetColorMask(const glm::bvec4 color_mask) { m_color_mask = color_mask; }
	void SetDepthStates(bool depth_test_enabled, bool depth_mask, GLenum depth_func);
	void SetStencilStates(bool stencil_enable, GLenum stencil_func, GLenum stencil_fail_op,
		GLenum depth_fail_op, GLenum pass_op, GLuint stencil_read_mask, GLuint stencil_write_mask,
		GLint stencil_ref);
	void SetCullFace(bool cull_face_enabled, GLenum cull_face_mode) { m_cull_face_enabled = cull_face_enabled;m_cull_face_mode = cull_face_mode; }
	void SetBlend(bool blend_enabled) { m_blend_enabled = blend_enabled; }
	const FramebufferPtr& GetFramebuffer() const { return m_fb; }
	void SetFramebuffer(const FramebufferPtr& fb) { m_fb = fb; }
	void AddRenderContext(RenderContext* rc)
	{
		m_rcs.emplace_back(rc);
	}

	void FlushRenderContext(bool clear_rcs)
	{
		ApplyFrameBuffer();
		ApplyRenderStates();
		for (auto& rc : m_rcs)
		{
			rc->Render(*this);
		}

		if (clear_rcs)
		{
			ClearRenderContext();
		}
	}

protected:
	void ApplyFrameBuffer();
	void ApplyRenderStates();

	glm::vec3 m_camera_position;
	glm::mat4 m_view;
	glm::mat4 m_proj;
	std::vector<RenderContext*> m_rcs;
	FramebufferPtr m_fb; // a nullptr represents the backbuffer

	// some view level render states
	GLenum m_polygon_mode = GL_FILL;
	glm::bvec4 m_color_mask = glm::bvec4(true);
	GLenum m_depth_func = GL_LESS;
	bool m_depth_test_enabled = true;
	bool m_depth_mask = true;

	// stencil states
	bool m_stencil_enable = false;
	GLenum m_stencil_func = GL_ALWAYS;
	GLenum m_stencil_fail_op = GL_KEEP;
	GLenum m_depth_fail_op = GL_KEEP;
	GLenum m_pass_op = GL_KEEP;
	GLuint m_stencil_read_mask = 0xFF;
	GLuint m_stencil_write_mask = 0xFF;
	GLint m_stencil_ref = 0;

	bool m_cull_face_enabled = false;
	GLenum m_cull_face_mode = GL_BACK;
	bool m_blend_enabled = false;
};


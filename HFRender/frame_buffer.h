#pragma once
#include <vector>
#include <cstdint>
#include "render_surface.h"
#include "texture.h"

class Framebuffer
{
public:
	Framebuffer(bool is_default = false);
	~Framebuffer();
	void AttachColorBuffer(RenderSurfacePtr&& color_surface);
	void AttachDepthBuffer(RenderSurfacePtr&& depth_surface);
	void AttachColorTexture(const Texture2DPtr& color_texture);
	void AttachDepthTexture(const Texture2DPtr& depth_texture);
	void AttachImage(const ITexturePtr& texture, GLenum access);
	bool CheckStatus();
	void Use();

	const glm::vec4& GetClearColor() const { return m_clear_color; }
	void SetClearColor(const glm::vec4& val) { m_clear_color = val; }
	float GetClearDepth() const { return m_clear_depth; }
	void SetClearDepth(float val) { m_clear_depth = val; }
	uint8_t GetClearStencil() const { return m_clear_stencil; }
	void SetClearStencil(uint8_t val) { m_clear_stencil = val; }

protected:
	GLuint m_id = 0;
	std::vector<RenderSurfacePtr> m_renderbuffers;
	std::vector<Texture2DPtr> m_texture2Ds;
	Texture2DPtr m_depth_texture;
	GLuint m_color_attachment = GL_COLOR_ATTACHMENT0;

	std::vector<ITexturePtr> m_images;

	glm::vec4 m_clear_color = glm::vec4(0.f, 0.f, 0.f, 0.f);
	float m_clear_depth = 1.f;
	uint8_t m_clear_stencil = 0;
};

typedef std::shared_ptr<Framebuffer> FramebufferPtr;

#include "EngineCommon/precomp.h"

#ifndef DISABLE_SDL

#include "HAL/Graphics/Renderer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glew/glew.h>

#include "EngineCommon/Debug/ConcurrentAccessDetector.h"

#include "HAL/Graphics/SdlSurface.h"


static constexpr double MATH_PI = 3.14159265358979323846;

#ifdef CONCURRENT_ACCESS_DETECTION
namespace HAL
{
	extern ConcurrentAccessDetector gSDLAccessDetector;
}
#endif

namespace Graphics
{
	void Render::BindSurface(const Surface& surface)
	{
		surface.bind();
	}

	void Render::DrawQuad(const glm::mat4& transform, const Vector2D size, const QuadUV uv, const float alpha)
	{
		DETECT_CONCURRENT_ACCESS(HAL::gSDLAccessDetector);
		glLoadMatrixf(reinterpret_cast<const float*>(&transform));

		glBegin(GL_QUADS);
		glColor4f(1.0f, 1.0f, 1.0f, alpha);
		glTexCoord2f(uv.u1, uv.v2); glVertex2f(0.0f, size.y);
		glTexCoord2f(uv.u2, uv.v2); glVertex2f(size.x, size.y);
		glTexCoord2f(uv.u2, uv.v1); glVertex2f(size.x, 0.0f);
		glTexCoord2f(uv.u1, uv.v1); glVertex2f(0.0f, 0.0f);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glEnd();
	}

	void Render::DrawQuad(const Vector2D pos, const Vector2D size)
	{
		DETECT_CONCURRENT_ACCESS(HAL::gSDLAccessDetector);
		glm::mat4 transform{ 1.0f };
		transform = glm::translate(transform, glm::vec3(pos.x, pos.y, 0.0f));
		glLoadMatrixf(reinterpret_cast<const float*>(&transform));

		glBegin(GL_QUADS);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, size.y);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(size.x, size.y);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(size.x, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glEnd();
	}

	void Render::DrawQuad(const Vector2D pos, const Vector2D size, const Vector2D anchor, const float rotation, const QuadUV uv, const float alpha)
	{
		glm::mat4 transform{ 1.0f };
		transform = glm::translate(transform, glm::vec3(pos.x, pos.y, 0.0f));
		transform = glm::rotate(transform, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
		transform = glm::translate(transform, glm::vec3(-size.x * anchor.x, -size.y * anchor.y, 0.0f));
		DrawQuad(transform, size, uv, alpha);
	}

	void Render::DrawFan(const std::vector<DrawPoint>& points, const glm::mat4& transform, const float alpha)
	{
		DETECT_CONCURRENT_ACCESS(HAL::gSDLAccessDetector);
		glLoadMatrixf(reinterpret_cast<const float*>(&transform));

		glBegin(GL_TRIANGLE_FAN);
		glColor4f(1.0f, 1.0f, 1.0f, alpha);
		for (const DrawPoint& point : points)
		{
			glTexCoord2f(point.texturePoint.x, point.texturePoint.y);
			glVertex2f(point.spacePoint.x, point.spacePoint.y);
		}
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glEnd();
	}

	void Render::DrawStrip(const std::vector<DrawPoint>& points, const glm::mat4& transform, const float alpha)
	{
		DETECT_CONCURRENT_ACCESS(HAL::gSDLAccessDetector);
		glLoadMatrixf(reinterpret_cast<const float*>(&transform));

		glBegin(GL_TRIANGLE_STRIP);
		glColor4f(1.0f, 1.0f, 1.0f, alpha);
		for (const DrawPoint& point : points)
		{
			glTexCoord2f(point.texturePoint.x, point.texturePoint.y);
			glVertex2f(point.spacePoint.x, point.spacePoint.y);
		}
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glEnd();
	}

	void Render::DrawTiledQuad(const Vector2D start, const Vector2D size, const QuadUV& uv)
	{
		DETECT_CONCURRENT_ACCESS(HAL::gSDLAccessDetector);
		glm::mat4 transform{ 1.0f };
		glLoadMatrixf(reinterpret_cast<const float*>(&transform));

		glBegin(GL_QUADS);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glTexCoord2f(uv.u1, uv.v2); glVertex2f(start.x, start.y + size.y);
		glTexCoord2f(uv.u2, uv.v2); glVertex2f(start.x + size.x, start.y + size.y);
		glTexCoord2f(uv.u2, uv.v1); glVertex2f(start.x + size.x, start.y);
		glTexCoord2f(uv.u1, uv.v1); glVertex2f(start.x, start.y);
		glEnd();
	}

	void Renderer::renderText(const Graphics::Font& /*font*/, Vector2D /*pos*/, Graphics::Color /*color*/, const char* /*text*/)
	{
		//FC_DrawColor(font.getRawFont(), mRenderer, pos.x, pos.y, { color.R, color.G, color.B, color.A }, text);
	}

	std::array<int, 2> Renderer::getTextSize(const Graphics::Font& /*font*/, const char* /*text*/)
	{
		//return { FC_GetWidth(font.getRawFont(), text), FC_GetHeight(font.getRawFont(), text) };
		return { 1, 1 };
	}
}

#endif // !DISABLE_SDL

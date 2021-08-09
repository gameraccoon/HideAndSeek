#include "Base/precomp.h"

#include "HAL/Graphics/Renderer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glew/glew.h>

#include "sdl/SDL_surface.h"

#include "Base/Debug/ConcurrentAccessDetector.h"

#include "HAL/Internal/GlContext.h"

#include "HAL/Internal/SdlWindow.h"
#include "HAL/Internal/SdlSurface.h"
#include "HAL/Graphics/Font.h"
#include "HAL/Internal/SdlSurface.h"


static constexpr double MATH_PI = 3.14159265358979323846;

#ifdef CONCURRENT_ACCESS_DETECTION
namespace HAL
{
	extern ConcurrentAccessDetector gSDLAccessDetector;
}
#endif

namespace Graphics
{
	void Render::DrawQuad(const Internal::Surface& surface, const glm::mat4& transform, Vector2D size, Graphics::QuadUV uv, float alpha)
	{
		DETECT_CONCURRENT_ACCESS(HAL::gSDLAccessDetector);
		glLoadMatrixf(reinterpret_cast<const float*>(&transform));
		surface.bind();

		glBegin(GL_QUADS);
		glColor4f(1.0f, 1.0f, 1.0f, alpha);
		glTexCoord2f(uv.u1, uv.v2); glVertex2f(0.0f, size.y);
		glTexCoord2f(uv.u2, uv.v2); glVertex2f(size.x, size.y);
		glTexCoord2f(uv.u2, uv.v1); glVertex2f(size.x, 0.0f);
		glTexCoord2f(uv.u1, uv.v1); glVertex2f(0.0f, 0.0f);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glEnd();
	}

	void Render::DrawQuad(const Internal::Surface& surface, Vector2D pos, Vector2D size)
	{
		DETECT_CONCURRENT_ACCESS(HAL::gSDLAccessDetector);
		glm::mat4 transform{ 1.0f };
		transform = glm::translate(transform, glm::vec3(pos.x, pos.y, 0.0f));
		glLoadMatrixf(reinterpret_cast<const float*>(&transform));
		surface.bind();

		glBegin(GL_QUADS);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, size.y);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(size.x, size.y);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(size.x, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glEnd();
	}

	void Render::DrawQuad(const Internal::Surface& surface, Vector2D pos, Vector2D size, Vector2D anchor, float rotation, Graphics::QuadUV uv, float alpha)
	{
		glm::mat4 transform{ 1.0f };
		transform = glm::translate(transform, glm::vec3(pos.x, pos.y, 0.0f));
		transform = glm::rotate(transform, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
		transform = glm::translate(transform, glm::vec3(-size.x * anchor.x, -size.y * anchor.y, 0.0f));
		DrawQuad(surface, transform, size, uv, alpha);
	}

	void Render::DrawFan(const Internal::Surface& surface, const std::vector<DrawPoint>& points, const glm::mat4& transform, float alpha)
	{
		DETECT_CONCURRENT_ACCESS(HAL::gSDLAccessDetector);
		glLoadMatrixf(reinterpret_cast<const float*>(&transform));
		surface.bind();

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

	void Render::DrawStrip(const Internal::Surface& surface, const std::vector<DrawPoint>& points, const glm::mat4& transform, float alpha)
	{
		DETECT_CONCURRENT_ACCESS(HAL::gSDLAccessDetector);
		glLoadMatrixf(reinterpret_cast<const float*>(&transform));
		surface.bind();

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

	void Render::DrawTiledQuad(const Internal::Surface& surface, Vector2D start, Vector2D size, const QuadUV& uv)
	{
		DETECT_CONCURRENT_ACCESS(HAL::gSDLAccessDetector);
		glm::mat4 transform{ 1.0f };
		glLoadMatrixf(reinterpret_cast<const float*>(&transform));
		surface.bind();

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

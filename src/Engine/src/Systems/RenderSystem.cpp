#include "Systems/RenderSystem.h"

#include <Components/RenderComponent.h>
#include <Components/TransformComponent.h>
#include <Components/CollisionComponent.h>
#include <Base/Engine.h>
#include <Modules/RayTrace.h>

// for tests only
#include "../src/Internal/SdlSurface.h"
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>

RenderSystem::RenderSystem(SystemInterface::Engine* engine, std::shared_ptr<SystemInterface::ResourceManager> resourceManager)
	: mEngine(engine)
#ifdef DEBUG
	, mDebugDrawer(resourceManager)
#endif // DEBUG
{
}

void RenderSystem::update(World* world, float /*dt*/)
{
	NullableEntity mainCamera = world->getMainCamera();
	if (!mainCamera.isValid())
	{
		return;
	}

	auto [cameraTransformComponent] = world->getEntityManger().getEntityComponents<TransformComponent>(mainCamera.getEntity());
	if (cameraTransformComponent == nullptr)
	{
		return;
	}

	Vector2D cameraLocation = cameraTransformComponent->getLocation();
	Vector2D mouseScreenPos(mEngine->getMouseX(), mEngine->getMouseY());
	Vector2D screenHalfSize = Vector2D(mEngine->getWidth(), mEngine->getHeight()) * 0.5f;

	Vector2D drawShift = screenHalfSize - cameraLocation + (screenHalfSize - mouseScreenPos) * 0.5;

	drawVisibilityPolygon(world, Vector2D(500.0f, 500.0f), drawShift);

	world->getEntityManger().forEachEntity<RenderComponent, TransformComponent>([&drawShift](RenderComponent* renderComponent, TransformComponent* transformComponent)
	{
		const auto& optionalTexture = renderComponent->getTexture();
		if (optionalTexture.has_value())
		{
			if (const Graphics::Texture& texture = optionalTexture.value(); texture.isValid())
			{
				auto location = transformComponent->getLocation() + drawShift;
				auto anchor = renderComponent->getAnchor();
				auto scale = renderComponent->getScale();
				texture.draw(location.x, location.y, anchor.x, anchor.y, scale.x, scale.y, transformComponent->getRotation().getValue(), 1.0f);
			}
		}
	});

#ifdef DEBUG
	mDebugDrawer.render(world, drawShift);
#endif // DEBUG
}

struct SimpleBorder
{
	Vector2D a;
	Vector2D b;
};

enum class PointSide
{
	Left, // the point is free from the left (counter-clockwise) side
	Rignt, // the point is free from the right (clockwise) side
	InBetween // the point is connecting two borders that are facing the light source
};

struct AngledPoint
{
	Vector2D coords;
	float angle;
	PointSide side;
};

static float CalcSignedArea(const Vector2D &a, const Vector2D &b, const Vector2D &c)
{
	return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

static float CalcClockwiseDirection(const Vector2D &a, const Vector2D &b)
{
	return a.x * (b.y - a.y) - a.y * (b.x - a.x);
}

static bool LessPointAngle(const AngledPoint& a, const AngledPoint& b)
{
	return a.angle < b.angle;
}

static void AddVisiblePoint(bool isVisible, bool isPreviousVisible, const Vector2D& a, std::vector<AngledPoint>& pointsToTrace)
{
	if (isVisible)
	{
		if (isPreviousVisible)
		{
			pointsToTrace.push_back({a, a.rotation().getValue(), PointSide::InBetween});
		}
		else
		{
			pointsToTrace.push_back({a, a.rotation().getValue(), PointSide::Rignt});
		}
	}
	else
	{
		if (isPreviousVisible)
		{
			pointsToTrace.push_back({a, a.rotation().getValue(), PointSide::Left});
		}
	}
}

void RenderSystem::drawVisibilityPolygon(World* world, const Vector2D& fowSize, const Vector2D& drawShift)
{
	NullableEntity playerEntity = world->getPlayerControlledEntity();

	if (!playerEntity.isValid())
	{
		return;
	}

	auto [playerTransform] = world->getEntityManger().getEntityComponents<TransformComponent>(playerEntity.getEntity());

	if (playerTransform == nullptr)
	{
		return;
	}

	Vector2D playerSightPosition = playerTransform->getLocation();

	const auto components = world->getEntityManger().getComponents<CollisionComponent, TransformComponent>();

	std::vector<SimpleBorder> borders;
	borders.reserve(components.size() * 4 + 4);
	std::vector<AngledPoint> pointsToTrace;
	pointsToTrace.reserve(components.size() * 4 + 4);

	// populate the borders of visible area
	float left = -fowSize.x * 0.5f;
	float right = fowSize.x * 0.5f;
	float top = -fowSize.y * 0.5f;
	float bottom = fowSize.y * 0.5f;
	Vector2D leftTop(left, top);
	Vector2D rightTop(right, top);
	Vector2D rightBottom(right, bottom);
	Vector2D leftBottom(left, bottom);
	pointsToTrace.push_back({leftTop, leftTop.rotation().getValue(), PointSide::InBetween});
	pointsToTrace.push_back({rightTop, rightTop.rotation().getValue(), PointSide::InBetween});
	pointsToTrace.push_back({rightBottom, rightBottom.rotation().getValue(), PointSide::InBetween});
	pointsToTrace.push_back({leftBottom, leftBottom.rotation().getValue(), PointSide::InBetween});
	borders.push_back({leftTop, rightTop});
	borders.push_back({rightTop, rightBottom});
	borders.push_back({rightBottom, leftBottom});
	borders.push_back({leftBottom, leftTop});

	// find borders that facing the light source and their points
	for (const auto& [collision, transform] : components)
	{
		Vector2D shift = transform->getLocation() - playerSightPosition;
		const Hull& hull = collision->getGeometry();
		bool isPreviousVisible = false;
		bool isNotFirst = false;
		bool isFirstVisible;
		for (const Border& border : hull.borders)
		{
			Vector2D a = border.getA() + shift;
			Vector2D b = border.getB() + shift;
			// keep only borders inside the draw area and facing the light source
			bool isVisible = ((a.x > left && a.x < right && a.y > top && a.y < bottom)
							  || (b.x > left && b.x < right && b.y > top && b.y < bottom)
							  ) && CalcClockwiseDirection(a, b) < 0.0f;

			if (isVisible)
			{
				borders.push_back({b, a});
			}

			if (isNotFirst)
			{
				AddVisiblePoint(isVisible, isPreviousVisible, a, pointsToTrace);
			}
			else
			{
				isFirstVisible = isVisible;
				isNotFirst = true;
			}
			isPreviousVisible = isVisible;
		}

		if (hull.borders.size() > 0)
		{
			Vector2D a = hull.borders[0].getA() + shift;
			AddVisiblePoint(isFirstVisible, isPreviousVisible, a, pointsToTrace);
		}
	}

	if (borders.size() == 0)
	{
		return;
	}

	// sort points that we can iterate over them in clockwise order
	std::sort(pointsToTrace.begin(), pointsToTrace.end(), LessPointAngle);

	std::vector<Vector2D> polygon;

	// calculate visibility polygon
	Vector2D nearestPoint;
	for (AngledPoint& point : pointsToTrace)
	{
		nearestPoint = point.coords;
		bool notFound = true;
		bool needToSkip = false;
		for (SimpleBorder& border : borders)
		{
			if (CalcClockwiseDirection(border.a, point.coords) > 0.0f
				&&
				CalcClockwiseDirection(point.coords, border.b) > 0.0f)
			{
				if (CalcSignedArea(border.a, border.b, point.coords) < 0)
				{
					// this point is hidden by some obstacle
					needToSkip = true;
					break;
				}

				if (point.side == PointSide::InBetween)
				{
					// don't trace through in-between points
					continue;
				}

				Vector2D intersectPoint = RayTrace::GetPointIntersect2Lines(border.a, border.b, ZERO_VECTOR, point.coords);
				if (notFound || intersectPoint.qSize() < nearestPoint.qSize())
				{
					nearestPoint = intersectPoint;
					notFound = false;
				}
			}
		}

		if (needToSkip)
		{
			continue;
		}

		// add point (and optionally its casted shadow) to the polygon
		if (nearestPoint != point.coords)
		{
			if (point.side == PointSide::Left)
			{
				polygon.push_back(nearestPoint);
				polygon.push_back(point.coords);
			}
			else if (point.side == PointSide::Rignt)
			{
				polygon.push_back(point.coords);
				polygon.push_back(nearestPoint);
			}
		}
		else
		{
			polygon.push_back(point.coords);
		}
	}

	// draw the polygon
	if (polygon.size() > 2)
	{
		std::vector<SystemInterface::DrawPoint> drawablePolygon;
		drawablePolygon.reserve(polygon.size() + 2);
		drawablePolygon.push_back(SystemInterface::DrawPoint{ZERO_VECTOR, Vector2D(0.5f, 0.5f)});
		for (auto& point : polygon)
		{
			drawablePolygon.push_back(SystemInterface::DrawPoint{point, Vector2D(0.5f-point.x/fowSize.x, 0.5f-point.y/fowSize.y)});
		}
		drawablePolygon.push_back(SystemInterface::DrawPoint{polygon[0], Vector2D(0.5f-polygon[0].x/fowSize.x, 0.5f-polygon[0].y/fowSize.y)});

		static SystemInterface::Internal::SdlSurface surface("resources/textures/light.png");
		glm::mat4 transform(1.0f);
		transform = glm::translate(transform, glm::vec3(playerSightPosition.x + drawShift.x, playerSightPosition.y + drawShift.y, 0.0f));
		mEngine->render(&surface, drawablePolygon, transform, 0.5f);
	}
}

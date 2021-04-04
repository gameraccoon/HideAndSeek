#include "Base/precomp.h"

#include "GameLogic/Systems/DebugDrawSystem.h"

#include "Base/Random/Random.h"

#include <algorithm>

#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/NavMeshComponent.generated.h"
#include "GameData/Components/RenderModeComponent.generated.h"
#include "GameData/Components/AiControllerComponent.generated.h"
#include "GameData/Components/CharacterStateComponent.generated.h"
#include "GameData/Components/WorldCachedDataComponent.generated.h"
#include "GameData/Components/DebugDrawComponent.generated.h"
#include "GameData/World.h"
#include "GameData/GameData.h"

#include "Utils/Geometry/VisibilityPolygon.h"

#include "HAL/Base/Engine.h"
#include "HAL/Base/Math.h"
#include "HAL/Graphics/Renderer.h"
#include "HAL/Graphics/Sprite.h"
#include "HAL/Graphics/Font.h"

#include <glm/gtc/matrix_transform.hpp>


DebugDrawSystem::DebugDrawSystem(WorldHolder& worldHolder, const TimeData& timeData, HAL::Engine& engine, HAL::ResourceManager& resourceManager)
	: mWorldHolder(worldHolder)
	, mTime(timeData)
	, mEngine(engine)
	, mResourceManager(resourceManager)
{
}

template<typename T>
void RemoveOldDrawElement(std::vector<T>& vector, GameplayTimestamp now)
{
	std::erase_if(
		vector,
		[now](const T& val){ return val.isLifeTimeExceeded(now); }
	);
}

static Vector2D GetNavmeshPolygonCenter(size_t triangleIdx, const NavMesh::Geometry& navMeshGeometry)
{
	Vector2D polygonCenter{ZERO_VECTOR};
	for (size_t i = 0; i < navMeshGeometry.vertsPerPoly; ++i)
	{
		polygonCenter += navMeshGeometry.vertices[navMeshGeometry.indexes[triangleIdx * navMeshGeometry.vertsPerPoly + i]];
	}
	polygonCenter /= static_cast<float>(navMeshGeometry.vertsPerPoly);
	return polygonCenter;
}

static void DrawPath(const std::vector<Vector2D>& path, const Graphics::Sprite& navMeshSprite, const Graphics::QuadUV& quadUV, Vector2D drawShift)
{
	if (path.size() > 1)
	{
		std::vector<Graphics::DrawPoint> drawablePolygon;
		drawablePolygon.reserve(path.size() * 2);

		{
			float u1 = static_cast<float>(Random::GlobalGenerator() * 1.0f / Random::GlobalGenerator.max());
			float v1 = static_cast<float>(Random::GlobalGenerator() * 1.0f / Random::GlobalGenerator.max());
			float u2 = static_cast<float>(Random::GlobalGenerator() * 1.0f / Random::GlobalGenerator.max());
			float v2 = static_cast<float>(Random::GlobalGenerator() * 1.0f / Random::GlobalGenerator.max());

			Vector2D normal = (path[1] - path[0]).normal() * 3;

			drawablePolygon.push_back(Graphics::DrawPoint{path[0] + normal, Graphics::QuadLerp(quadUV, u1, v1)});
			drawablePolygon.push_back(Graphics::DrawPoint{path[0] - normal, Graphics::QuadLerp(quadUV, u2, v2)});
		}

		for (size_t i = 1; i < path.size(); ++i)
		{
			float u1 = static_cast<float>(Random::GlobalGenerator() * 1.0f / Random::GlobalGenerator.max());
			float v1 = static_cast<float>(Random::GlobalGenerator() * 1.0f / Random::GlobalGenerator.max());
			float u2 = static_cast<float>(Random::GlobalGenerator() * 1.0f / Random::GlobalGenerator.max());
			float v2 = static_cast<float>(Random::GlobalGenerator() * 1.0f / Random::GlobalGenerator.max());

			Vector2D normal = (path[i] - path[i-1]).normal() * 3;

			drawablePolygon.push_back(Graphics::DrawPoint{path[i] + normal, Graphics::QuadLerp(quadUV, u1, v1)});
			drawablePolygon.push_back(Graphics::DrawPoint{path[i] - normal, Graphics::QuadLerp(quadUV, u2, v2)});
		}

		glm::mat4 transform(1.0f);
		transform = glm::translate(transform, glm::vec3(drawShift.x, drawShift.y, 0.0f));
		Graphics::Render::drawStrip(*navMeshSprite.getSurface(), drawablePolygon, transform, 0.5f);
	}
}

void DebugDrawSystem::update()
{
	World& world = mWorldHolder.getWorld();
	GameData& gameData = mWorldHolder.getGameData();
	Graphics::Renderer& renderer = mEngine.getRenderer();

	auto [worldCachedData] = world.getWorldComponents().getComponents<WorldCachedDataComponent>();
	Vector2D workingRect = worldCachedData->getScreenSize();
	Vector2D cameraLocation = worldCachedData->getCameraPos();
	CellPos cameraCell = worldCachedData->getCameraCellPos();

	Vector2D screenHalfSize = mEngine.getWindowSize() * 0.5f;

	Vector2D drawShift = screenHalfSize - cameraLocation;

	SpatialEntityManager spatialManager = world.getSpatialData().getCellManagersAround(cameraLocation, workingRect);

	auto [renderMode] = gameData.getGameComponents().getComponents<RenderModeComponent>();

	if (renderMode && renderMode->getIsDrawDebugCellInfoEnabled())
	{
		const Graphics::Font& font = mResourceManager.getResource<Graphics::Font>(mFontHandle);
		const Graphics::Sprite& collisionSprite = mResourceManager.getResource<Graphics::Sprite>(mCollisionSpriteHandle);
		Graphics::QuadUV quadUV = collisionSprite.getUV();

		std::vector<WorldCell*> cellsAround = world.getSpatialData().getCellsAround(cameraLocation, screenHalfSize*2.0f);

		for (WorldCell* cell : cellsAround)
		{
			CellPos cellPos = cell->getPos();
			Vector2D location = SpatialWorldData::GetRelativeLocation(cameraCell, cellPos, drawShift);
			Graphics::Render::drawQuad(*collisionSprite.getSurface(),
				location,
				SpatialWorldData::CellSizeVector,
				ZERO_VECTOR,
				0.0f,
				quadUV);

			std::string text = FormatString("(%d, %d)", cellPos.x, cellPos.y);
			std::array<int, 2> textSize = renderer.getTextSize(font, text.c_str());
			Vector2D screenPos = SpatialWorldData::CellSizeVector*0.5 + SpatialWorldData::GetCellRealDistance(cellPos - cameraCell) - cameraLocation + screenHalfSize - Vector2D(textSize[0] * 0.5f, textSize[1] * 0.5f);
			renderer.renderText(font, screenPos, {255, 255, 255, 255}, text.c_str());
		}
	}

	if (renderMode && renderMode->getIsDrawDebugCollisionsEnabled())
	{
		const Graphics::Sprite& collisionSprite = mResourceManager.getResource<Graphics::Sprite>(mCollisionSpriteHandle);
		Graphics::QuadUV quadUV = collisionSprite.getUV();
		spatialManager.forEachComponentSet<CollisionComponent, TransformComponent>([&collisionSprite, &quadUV, drawShift, &renderer, cameraCell](CollisionComponent* collision, TransformComponent* transform)
		{
			Vector2D location = transform->getLocation() + drawShift;
			Graphics::Render::drawQuad(*collisionSprite.getSurface(),
				Vector2D(collision->getBoundingBox().minX + location.x, collision->getBoundingBox().minY + location.y),
				Vector2D(collision->getBoundingBox().maxX-collision->getBoundingBox().minX,
						 collision->getBoundingBox().maxY-collision->getBoundingBox().minY),
				ZERO_VECTOR,
				0.0f,
				quadUV);
			return true;
		});
	}

	if (renderMode && renderMode->getIsDrawDebugAiDataEnabled())
	{
		const Graphics::Sprite& navMeshSprite = mResourceManager.getResource<Graphics::Sprite>(mNavmeshSpriteHandle);
		Graphics::QuadUV quadUV = navMeshSprite.getUV();
		auto [navMeshComponent] = world.getWorldComponents().getComponents<NavMeshComponent>();

		if (navMeshComponent)
		{
			const NavMesh& navMesh = navMeshComponent->getNavMesh();
			const NavMesh::Geometry& navMeshGeometry = navMesh.geometry;
			std::vector<Graphics::DrawPoint> drawablePolygon;
			drawablePolygon.reserve(navMeshGeometry.vertsPerPoly);
			for (size_t k = 0; k < navMeshGeometry.polygonsCount; ++k)
			{
				drawablePolygon.clear();
				for (size_t j = 0; j < navMeshGeometry.vertsPerPoly; ++j)
				{
					float u = static_cast<float>(Random::GlobalGenerator() * 1.0f / Random::GlobalGenerator.max());
					float v = static_cast<float>(Random::GlobalGenerator() * 1.0f / Random::GlobalGenerator.max());

					Vector2D pos = navMeshGeometry.vertices[navMeshGeometry.indexes[k*navMeshGeometry.vertsPerPoly + j]];
					drawablePolygon.push_back(Graphics::DrawPoint{pos, Graphics::QuadLerp(quadUV, u, v)});
				}
				glm::mat4 transform(1.0f);
				transform = glm::translate(transform, glm::vec3(drawShift.x, drawShift.y, 0.0f));
				Graphics::Render::drawFan(*navMeshSprite.getSurface(), drawablePolygon, transform, 0.3f);
			}

			const NavMesh::InnerLinks& navMeshLinks = navMesh.links;
			for (size_t i = 0; i < navMeshLinks.links.size(); ++i)
			{
				Vector2D firstPolygonCenter = GetNavmeshPolygonCenter(i, navMeshGeometry);
				for (NavMesh::InnerLinks::LinkData link : navMeshLinks.links[i])
				{
					Vector2D secondPolygonCenter = GetNavmeshPolygonCenter(link.neighbor, navMeshGeometry);
					std::vector<Vector2D> line{firstPolygonCenter, secondPolygonCenter + (firstPolygonCenter - secondPolygonCenter)*0.52f};
					DrawPath(line, navMeshSprite, quadUV, drawShift);
				}
			}
		}

		spatialManager.forEachComponentSet<AiControllerComponent>([&navMeshSprite, &quadUV, drawShift](AiControllerComponent* aiController)
		{
			DrawPath(aiController->getPathRef().smoothPath, navMeshSprite, quadUV, drawShift);
		});
	}

	if (renderMode && renderMode->getIsDrawDebugPrimitivesEnabled())
	{
		auto [debugDraw] = gameData.getGameComponents().getComponents<DebugDrawComponent>();
		if (debugDraw != nullptr)
		{
			Vector2D pointSize(6, 6);
			const Graphics::Sprite& pointSprite = mResourceManager.getResource<Graphics::Sprite>(mPointTextureHandle);
			const Graphics::Sprite& lineSprite = mResourceManager.getResource<Graphics::Sprite>(mLineTextureHandle);
			const Graphics::Font& font = mResourceManager.getResource<Graphics::Font>(mFontHandle);
			for (const auto& screenPoint : debugDraw->getScreenPoints())
			{
				Graphics::Render::drawQuad(*pointSprite.getSurface(), screenPoint.screenPos, pointSize);
				if (!screenPoint.name.empty())
				{
					renderer.renderText(font, screenPoint.screenPos, {255, 255, 255, 255}, screenPoint.name.c_str());
				}
			}

			for (const auto& worldPoint : debugDraw->getWorldPoints())
			{
				Vector2D screenPos = worldPoint.pos - cameraLocation + screenHalfSize;
				Graphics::Render::drawQuad(*pointSprite.getSurface(), screenPos, pointSize);
				if (!worldPoint.name.empty())
				{
					renderer.renderText(font, screenPos, {255, 255, 255, 255}, worldPoint.name.c_str());
				}
			}

			for (const auto& worldLineSegment : debugDraw->getWorldLineSegments())
			{
				Vector2D screenPosStart = worldLineSegment.startPos - cameraLocation + screenHalfSize;
				Vector2D screenPosEnd = worldLineSegment.endPos - cameraLocation + screenHalfSize;
				Vector2D diff = screenPosEnd - screenPosStart;
				Graphics::Render::drawQuad(
					*lineSprite.getSurface(),
					(screenPosStart + screenPosEnd) * 0.5f,
					Vector2D(diff.size(), pointSize.y),
					Vector2D(0.5f, 0.5f),
					diff.rotation().getValue(),
					lineSprite.getUV()
				);
			}
		}
	}

	if (renderMode && renderMode->getIsDrawDebugCharacterInfoEnabled())
	{
		const Graphics::Font& font = mResourceManager.getResource<Graphics::Font>(mFontHandle);
		spatialManager.forEachComponentSet<CharacterStateComponent, TransformComponent>([&renderer, &font, drawShift, cameraCell](CharacterStateComponent* characterState, TransformComponent* transform)
		{
			Vector2D location = transform->getLocation() + drawShift;
			renderer.renderText(font, location, {255, 255, 255, 255}, ID_TO_STR(enum_to_string(characterState->getState())).c_str());
		});
	}

	auto [debugDraw] = gameData.getGameComponents().getComponents<DebugDrawComponent>();
	if (debugDraw != nullptr)
	{
		RemoveOldDrawElement(debugDraw->getWorldPointsRef(), mTime.currentTimestamp);
		RemoveOldDrawElement(debugDraw->getScreenPointsRef(), mTime.currentTimestamp);
		RemoveOldDrawElement(debugDraw->getWorldLineSegmentsRef(), mTime.currentTimestamp);
	}
}

void DebugDrawSystem::initResources()
{
	mCollisionSpriteHandle = mResourceManager.lockSprite("resources/textures/collision.png");
	mNavmeshSpriteHandle = mResourceManager.lockSprite("resources/textures/testTexture.png");
	mPointTextureHandle = mResourceManager.lockSprite("resources/textures/collision.png");
	mLineTextureHandle = mResourceManager.lockSprite("resources/textures/testTexture.png");
	mFontHandle = mResourceManager.lockFont("resources/fonts/prstart.ttf", 16);
}

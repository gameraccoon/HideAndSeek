#include "GameLogic/Systems/DebugDrawSystem.h"

#include "GameData/Components/RenderComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/NavMeshComponent.generated.h"
#include "GameData/Components/RenderModeComponent.generated.h"
#include "GameData/Components/AiControllerComponent.generated.h"
#include "GameData/World.h"

#include "Utils/Geometry/VisibilityPolygon.h"

#include "HAL/Base/Alghorithms.h"
#include "HAL/Base/Engine.h"
#include "HAL/Internal/SdlSurface.h"

#include <glm/gtc/matrix_transform.hpp>

#include <DetourNavMesh.h>


DebugDrawSystem::DebugDrawSystem(WorldHolder& worldHolder, HAL::Engine* engine, const std::shared_ptr<HAL::ResourceManager>& resourceManager)
	: mWorldHolder(worldHolder)
	, mEngine(engine)
	, mResourceManager(resourceManager)
{
	mCollisionSpriteHandle = resourceManager->lockSprite("resources/textures/collision.png");
	mNavmeshSpriteHandle = resourceManager->lockSprite("resources/textures/testTexture.png");
}

void DebugDrawSystem::update()
{
	World* world = mWorldHolder.world;

	static const Vector2D maxFov(500.0f, 500.0f);

	OptionalEntity mainCamera = world->getMainCamera();
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
	Vector2D screenHalfSize = Vector2D(static_cast<float>(mEngine->getWidth()), static_cast<float>(mEngine->getHeight())) * 0.5f;

	Vector2D drawShift = screenHalfSize - cameraLocation + (screenHalfSize - mouseScreenPos) * 0.5;

	auto [renderMode] = world->getWorldComponents().getComponents<RenderModeComponent>();
	if (renderMode && renderMode->getIsDrawDebugCollisionsEnabled())
	{
		const Graphics::Sprite& collisionSprite = mResourceManager->getSprite(mCollisionSpriteHandle);
		Graphics::QuadUV quadUV = collisionSprite.getUV();
		world->getEntityManger().forEachComponentSet<CollisionComponent>([&collisionSprite, &quadUV, drawShift, engine = mEngine](CollisionComponent* collisionComponent)
		{
			engine->render(collisionSprite.getSurface(),
				Vector2D(collisionComponent->getBoundingBox().minX + drawShift.x, collisionComponent->getBoundingBox().minY + drawShift.y),
				Vector2D(collisionComponent->getBoundingBox().maxX-collisionComponent->getBoundingBox().minX,
						 collisionComponent->getBoundingBox().maxY-collisionComponent->getBoundingBox().minY),
				ZERO_VECTOR,
				quadUV);
			return true;
		});
	}

	if (renderMode && renderMode->getIsDrawDebugNavmeshEnabled())
	{
		const Graphics::Sprite& navMeshSprite = mResourceManager->getSprite(mNavmeshSpriteHandle);
		Graphics::QuadUV quadUV = navMeshSprite.getUV();
		auto [navMeshComponent] = world->getWorldComponents().getComponents<NavMeshComponent>();

		if (navMeshComponent)
		{
			if (const dtNavMesh* navmesh = navMeshComponent->getNavMeshRef().getMesh())
			{
				int maxTiles = navmesh->getMaxTiles();
				for (int i = 0; i < maxTiles; ++i)
				{
					if (const dtMeshTile* tile = navmesh->getTile(i); tile && tile->header)
					{
						std::vector<std::array<Vector2D, 3>> points;
						for (int i = 0; i < tile->header->polyCount; ++i)
						{
							const auto& poly = tile->polys[i];
							std::vector<HAL::DrawPoint> drawablePolygon;
							drawablePolygon.reserve(3);
							for (int j = 0; j < poly.vertCount; ++j)
							{
								float u = static_cast<float>(std::rand() * 1.0f / RAND_MAX);
								float v = static_cast<float>(std::rand() * 1.0f / RAND_MAX);

								float x = tile->verts[poly.verts[j] * 3];
								float y = tile->verts[poly.verts[j] * 3 + 2];
								drawablePolygon.push_back(HAL::DrawPoint{Vector2D(x, y), Graphics::QuadLerp(quadUV, u, v)});
							}
							glm::mat4 transform(1.0f);
							transform = glm::translate(transform, glm::vec3(drawShift.x, drawShift.y, 0.0f));
							mEngine->renderFan(navMeshSprite.getSurface(), drawablePolygon, transform, 0.3f);
						}
					}
				}
			}
		}
	}

	if (renderMode && renderMode->getIsDrawDebugAiPathsEnabled())
	{
		const Graphics::Sprite& navMeshSprite = mResourceManager->getSprite(mNavmeshSpriteHandle);
		Graphics::QuadUV quadUV = navMeshSprite.getUV();
		world->getEntityManger().forEachComponentSet<AiControllerComponent>([drawShift, &quadUV, &navMeshSprite, engine = mEngine](AiControllerComponent* aiController)
		{
			std::vector<Vector2D>& path = aiController->getPathRef().getSmoothPathRef();
			if (path.size() > 1)
			{
				std::vector<HAL::DrawPoint> drawablePolygon;
				drawablePolygon.reserve(path.size() * 2);

				{
					float u1 = static_cast<float>(std::rand() * 1.0f / RAND_MAX);
					float v1 = static_cast<float>(std::rand() * 1.0f / RAND_MAX);
					float u2 = static_cast<float>(std::rand() * 1.0f / RAND_MAX);
					float v2 = static_cast<float>(std::rand() * 1.0f / RAND_MAX);

					Vector2D normal = (path[1] - path[0]).normal() * 3;

					drawablePolygon.push_back(HAL::DrawPoint{path[0] + normal, Graphics::QuadLerp(quadUV, u1, v1)});
					drawablePolygon.push_back(HAL::DrawPoint{path[0] - normal, Graphics::QuadLerp(quadUV, u2, v2)});
				}

				for (size_t i = 1; i < path.size(); ++i)
				{
					float u1 = static_cast<float>(std::rand() * 1.0f / RAND_MAX);
					float v1 = static_cast<float>(std::rand() * 1.0f / RAND_MAX);
					float u2 = static_cast<float>(std::rand() * 1.0f / RAND_MAX);
					float v2 = static_cast<float>(std::rand() * 1.0f / RAND_MAX);

					Vector2D normal = (path[i] - path[i-1]).normal() * 3;

					drawablePolygon.push_back(HAL::DrawPoint{path[i] + normal, Graphics::QuadLerp(quadUV, u1, v1)});
					drawablePolygon.push_back(HAL::DrawPoint{path[i] - normal, Graphics::QuadLerp(quadUV, u2, v2)});
				}

				glm::mat4 transform(1.0f);
				transform = glm::translate(transform, glm::vec3(drawShift.x, drawShift.y, 0.0f));
				engine->renderStrip(navMeshSprite.getSurface(), drawablePolygon, transform, 0.5f);
			}
		});
	}

	if (renderMode && renderMode->getIsDrawDebugFpsEnabled())
	{

	}
}

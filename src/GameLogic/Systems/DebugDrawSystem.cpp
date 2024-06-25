#include "EngineCommon/precomp.h"

#include "GameLogic/Systems/DebugDrawSystem.h"

#include <algorithm>

#include "EngineCommon/Random/Random.h"
#include "EngineCommon/Types/TemplateHelpers.h"

#include "GameData/Components/AiControllerComponent.generated.h"
#include "GameData/Components/CharacterStateComponent.generated.h"
#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/DebugDrawComponent.generated.h"
#include "GameData/Components/NavMeshComponent.generated.h"
#include "GameData/Components/RenderAccessorComponent.generated.h"
#include "GameData/Components/RenderModeComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/WorldCachedDataComponent.generated.h"
#include "GameData/Components/TimeComponent.generated.h"
#include "GameData/GameData.h"
#include "GameData/World.h"

#include "HAL/Graphics/Font.h"
#include "HAL/Graphics/Sprite.h"

#include "EngineLogic/Render/RenderAccessor.h"

DebugDrawSystem::DebugDrawSystem(
		WorldHolder& worldHolder,
		ResourceManager& resourceManager) noexcept
	: mWorldHolder(worldHolder)
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

static void DrawPath(RenderData& renderData, const std::vector<Vector2D>& path, const ResourceHandle& navMeshSprite, const Vector2D drawShift)
{
	if (path.size() > 1)
	{
		StripRenderData& stripData = TemplateHelpers::EmplaceVariant<StripRenderData>(renderData.layers);

		stripData.points.reserve(path.size() * 2);
		stripData.spriteHandle = navMeshSprite;
		stripData.drawShift = drawShift;
		stripData.alpha = 0.5f;

		{
			float u1 = static_cast<float>(Random::gGlobalGenerator()) * 1.0f / static_cast<float>(Random::GlobalGeneratorType::max());
			float v1 = static_cast<float>(Random::gGlobalGenerator()) * 1.0f / static_cast<float>(Random::GlobalGeneratorType::max());
			float u2 = static_cast<float>(Random::gGlobalGenerator()) * 1.0f / static_cast<float>(Random::GlobalGeneratorType::max());
			float v2 = static_cast<float>(Random::gGlobalGenerator()) * 1.0f / static_cast<float>(Random::GlobalGeneratorType::max());

			const Vector2D normal = (path[1] - path[0]).normal() * 3;

			stripData.points.push_back(Graphics::DrawPoint{path[0] + normal, {u1, v1}});
			stripData.points.push_back(Graphics::DrawPoint{path[0] - normal, {u2, v2}});
		}

		for (size_t i = 1; i < path.size(); ++i)
		{
			float u1 = static_cast<float>(Random::gGlobalGenerator()) * 1.0f / static_cast<float>(Random::GlobalGeneratorType::max());
			float v1 = static_cast<float>(Random::gGlobalGenerator()) * 1.0f / static_cast<float>(Random::GlobalGeneratorType::max());
			float u2 = static_cast<float>(Random::gGlobalGenerator()) * 1.0f / static_cast<float>(Random::GlobalGeneratorType::max());
			float v2 = static_cast<float>(Random::gGlobalGenerator()) * 1.0f / static_cast<float>(Random::GlobalGeneratorType::max());

			const Vector2D normal = (path[i] - path[i-1]).normal() * 3;

			stripData.points.push_back(Graphics::DrawPoint{path[i] + normal, {u1, v1}});
			stripData.points.push_back(Graphics::DrawPoint{path[i] - normal, {u2, v2}});
		}
	}
}

void DebugDrawSystem::update()
{
	SCOPED_PROFILER("DebugDrawSystem::update");
	World& world = mWorldHolder.getWorld();
	GameData& gameData = mWorldHolder.getGameData();

	const auto [time] = world.getWorldComponents().getComponents<const TimeComponent>();
	const TimeData& timeValue = time->getValue();

	auto [worldCachedData] = world.getWorldComponents().getComponents<WorldCachedDataComponent>();
	const Vector2D workingRect = worldCachedData->getScreenSize();
	const Vector2D cameraLocation = worldCachedData->getCameraPos();
	const CellPos cameraCell = worldCachedData->getCameraCellPos();

	SpatialEntityManager spatialManager = world.getSpatialData().getCellManagersAround(cameraLocation, workingRect);

	auto [renderMode] = gameData.getGameComponents().getComponents<const RenderModeComponent>();

	const Vector2D screenHalfSize = workingRect * 0.5f;
	const Vector2D drawShift = screenHalfSize - cameraLocation;

	auto [renderAccessorCmp] = gameData.getGameComponents().getComponents<RenderAccessorComponent>();
	if (renderAccessorCmp == nullptr || !renderAccessorCmp->getAccessor().has_value())
	{
		return;
	}

	const RenderAccessorGameRef renderAccessor = *renderAccessorCmp->getAccessor();

	std::unique_ptr<RenderData> renderData = std::make_unique<RenderData>();

	if (renderMode && renderMode->getIsDrawDebugCellInfoEnabled())
	{
		SpatialEntityManager::RecordsVector cellsAround = world.getSpatialData().getCellsAround(cameraLocation, screenHalfSize*2.0f);

		for (SpatialEntityManager::Record& record : cellsAround)
		{
			CellPos cellPos = record.extraData.get().getPos();
			const Vector2D location = SpatialWorldData::GetRelativeLocation(cameraCell, cellPos, drawShift);
			QuadRenderData& quadData = TemplateHelpers::EmplaceVariant<QuadRenderData>(renderData->layers);
			quadData.position = location;
			quadData.size = SpatialWorldData::CellSizeVector;
			quadData.spriteHandle = mCollisionSpriteHandle;

			TextRenderData& textData = TemplateHelpers::EmplaceVariant<TextRenderData>(renderData->layers);
			textData.color = {255, 255, 255, 255};
			textData.fontHandle = mFontHandle;
			textData.pos = SpatialWorldData::CellSizeVector*0.5 + SpatialWorldData::GetCellRealDistance(cellPos - cameraCell) - cameraLocation + screenHalfSize;
			textData.text = FormatString("(%d, %d)", cellPos.x, cellPos.y);
		}
	}

	if (renderMode && renderMode->getIsDrawDebugCollisionsEnabled())
	{
		spatialManager.forEachComponentSet<const CollisionComponent, const TransformComponent>(
			[&renderData, &collisionSpriteHandle = mCollisionSpriteHandle, drawShift](const CollisionComponent* collision, const TransformComponent* transform)
		{
			const Vector2D location = transform->getLocation() + drawShift;
			QuadRenderData& quadData = TemplateHelpers::EmplaceVariant<QuadRenderData>(renderData->layers);
			quadData.position = Vector2D(collision->getBoundingBox().minX + location.x, collision->getBoundingBox().minY + location.y);
			quadData.rotation = 0.0f;
			quadData.size = Vector2D(collision->getBoundingBox().maxX-collision->getBoundingBox().minX,
				collision->getBoundingBox().maxY-collision->getBoundingBox().minY);
			quadData.spriteHandle = collisionSpriteHandle;
			quadData.anchor = ZERO_VECTOR;
		});
	}

	if (renderMode && renderMode->getIsDrawDebugAiDataEnabled())
	{
		auto [navMeshComponent] = world.getWorldComponents().getComponents<NavMeshComponent>();

		if (navMeshComponent)
		{
			const NavMesh& navMesh = navMeshComponent->getNavMesh();
			const NavMesh::Geometry& navMeshGeometry = navMesh.geometry;
			for (size_t k = 0; k < navMeshGeometry.polygonsCount; ++k)
			{
				PolygonRenderData& polygon = TemplateHelpers::EmplaceVariant<PolygonRenderData>(renderData->layers);
				polygon.points.reserve(navMeshGeometry.verticesPerPoly);
				for (size_t j = 0; j < navMeshGeometry.verticesPerPoly; ++j)
				{
					float u = static_cast<float>(Random::gGlobalGenerator()) * 1.0f / static_cast<float>(Random::GlobalGeneratorType::max());
					float v = static_cast<float>(Random::gGlobalGenerator()) * 1.0f / static_cast<float>(Random::GlobalGeneratorType::max());

					const Vector2D pos = navMeshGeometry.vertices[navMeshGeometry.indexes[k*navMeshGeometry.verticesPerPoly + j]];
					polygon.points.push_back(Graphics::DrawPoint{pos, {u, v}});
				}

				polygon.spriteHandle = mNavmeshSpriteHandle;
				polygon.alpha = 0.3f;
				polygon.drawShift = drawShift;
			}
		}

		spatialManager.forEachComponentSet<const AiControllerComponent>(
			[&navMeshSpriteHandle = mNavmeshSpriteHandle, &renderData, drawShift](const AiControllerComponent* aiController)
		{
			DrawPath(*renderData, aiController->getPath().smoothPath, navMeshSpriteHandle, drawShift);
		});
	}

	if (renderMode && renderMode->getIsDrawDebugPrimitivesEnabled())
	{
		auto [debugDraw] = gameData.getGameComponents().getComponents<const DebugDrawComponent>();
		if (debugDraw != nullptr)
		{
			Vector2D pointSize(6, 6);
			for (const auto& screenPoint : debugDraw->getScreenPoints())
			{
				QuadRenderData& quadData = TemplateHelpers::EmplaceVariant<QuadRenderData>(renderData->layers);
				quadData.position = screenPoint.screenPos;
				quadData.size = pointSize;
				quadData.spriteHandle = mPointTextureHandle;
				if (!screenPoint.name.empty())
				{
					TextRenderData& textData = TemplateHelpers::EmplaceVariant<TextRenderData>(renderData->layers);
					textData.color = {255, 255, 255, 255};
					textData.fontHandle = mFontHandle;
					textData.pos = screenPoint.screenPos;
					textData.text = screenPoint.name;
				}
			}

			for (const auto& worldPoint : debugDraw->getWorldPoints())
			{
				const Vector2D screenPos = worldPoint.pos - cameraLocation + screenHalfSize;

				QuadRenderData& quadData = TemplateHelpers::EmplaceVariant<QuadRenderData>(renderData->layers);
				quadData.position = screenPos;
				quadData.rotation = 0.0f;
				quadData.size = pointSize;
				quadData.spriteHandle = mPointTextureHandle;
				if (!worldPoint.name.empty())
				{
					TextRenderData& textData = TemplateHelpers::EmplaceVariant<TextRenderData>(renderData->layers);
					textData.color = {255, 255, 255, 255};
					textData.fontHandle = mFontHandle;
					textData.pos = screenPos;
					textData.text = worldPoint.name;
				}
			}

			for (const auto& worldLineSegment : debugDraw->getWorldLineSegments())
			{
				QuadRenderData& quadData = TemplateHelpers::EmplaceVariant<QuadRenderData>(renderData->layers);
				Vector2D screenPosStart = worldLineSegment.startPos - cameraLocation + screenHalfSize;
				Vector2D screenPosEnd = worldLineSegment.endPos - cameraLocation + screenHalfSize;
				Vector2D diff = screenPosEnd - screenPosStart;
				quadData.position = (screenPosStart + screenPosEnd) * 0.5f;
				quadData.rotation = diff.rotation().getValue();
				quadData.size = {diff.size(), pointSize.y};
				quadData.spriteHandle = mLineTextureHandle;
			}
		}
	}

	if (renderMode && renderMode->getIsDrawDebugCharacterInfoEnabled())
	{
		spatialManager.forEachComponentSet<const CharacterStateComponent, const TransformComponent>(
			[&renderData, fontHandle = mFontHandle, drawShift](const CharacterStateComponent* characterState, const TransformComponent* transform)
		{
			TextRenderData& textData = TemplateHelpers::EmplaceVariant<TextRenderData>(renderData->layers);
			textData.color = {255, 255, 255, 255};
			textData.fontHandle = fontHandle;
			textData.pos = transform->getLocation() + drawShift;
			textData.text = ID_TO_STR(enum_to_string(characterState->getState()));
		});
	}

	auto [debugDraw] = gameData.getGameComponents().getComponents<DebugDrawComponent>();
	if (debugDraw != nullptr)
	{
		RemoveOldDrawElement(debugDraw->getWorldPointsRef(), timeValue.lastFixedUpdateTimestamp);
		RemoveOldDrawElement(debugDraw->getScreenPointsRef(), timeValue.lastFixedUpdateTimestamp);
		RemoveOldDrawElement(debugDraw->getWorldLineSegmentsRef(), timeValue.lastFixedUpdateTimestamp);
	}

	renderAccessor.submitData(std::move(renderData));
}

void DebugDrawSystem::init()
{
	SCOPED_PROFILER("DebugDrawSystem::initResources");
	mCollisionSpriteHandle = mResourceManager.lockResource<Graphics::Sprite>(RelativeResourcePath("resources/textures/collision.png"));
	mNavmeshSpriteHandle = mResourceManager.lockResource<Graphics::Sprite>(RelativeResourcePath("resources/textures/testTexture.png"));
	mPointTextureHandle = mResourceManager.lockResource<Graphics::Sprite>(RelativeResourcePath("resources/textures/collision.png"));
	mLineTextureHandle = mResourceManager.lockResource<Graphics::Sprite>(RelativeResourcePath("resources/textures/testTexture.png"));
	mFontHandle = mResourceManager.lockResource<Graphics::Font>(mResourceManager.getAbsoluteResourcePath(RelativeResourcePath("resources/fonts/prstart.ttf")), 16);
}

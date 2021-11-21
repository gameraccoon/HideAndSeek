#include "Base/precomp.h"

#include "AutoTests/Tests/CollidingCircularUnits/TestCase.h"

#include <memory>

#include "HAL/Base/Engine.h"

#include "GameData/Spatial/SpatialWorldData.h"

#include "GameData/Components/SpriteCreatorComponent.generated.h"
#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/StateMachineComponent.generated.h"
#include "GameData/Components/MovementComponent.generated.h"

#include "GameLogic/Systems/RenderSystem.h"
#include "GameLogic/Systems/CollisionSystem.h"
#include "GameLogic/Systems/ResourceStreamingSystem.h"
#include "GameLogic/Systems/MovementSystem.h"
#include "GameLogic/Systems/CharacterStateSystem.h"
#include "GameLogic/Systems/CameraSystem.h"

#include "AutoTests/Tests/CollidingCircularUnits/Systems/TestCircularUnitsSystem.h"
#include "AutoTests/Tests/CollidingCircularUnits/Systems/TestUnitsCountControlSystem.h"

void CollidingCircularUnitsTestCase::initTestCase(const ArgumentsParser& arguments)
{
	getResourceManager().loadAtlasesData("resources/atlas/atlas-list.json");

	getSystemsManager().registerSystem<TestUnitsCountControlSystem,
		RaccoonEcs::EntityAdder,
		RaccoonEcs::ComponentAdder<TransformComponent>,
		RaccoonEcs::ComponentAdder<MovementComponent>,
		RaccoonEcs::ComponentAdder<SpriteCreatorComponent>,
		RaccoonEcs::ComponentAdder<CollisionComponent>,
		RaccoonEcs::ComponentAdder<AiControllerComponent>,
		RaccoonEcs::ComponentAdder<CharacterStateComponent>>(
		RaccoonEcs::SystemDependencies(),
		getWorldHolder()
	);

	getSystemsManager().registerSystem<TestCircularUnitsSystem,
		RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent>,
		RaccoonEcs::ComponentFilter<const TransformComponent>,
		RaccoonEcs::ComponentFilter<const AiControllerComponent, const TransformComponent, MovementComponent>>(
		RaccoonEcs::SystemDependencies().goesAfter<TestUnitsCountControlSystem>(),
		getWorldHolder(),
		getTime()
	);

	getSystemsManager().registerSystem<CollisionSystem,
		RaccoonEcs::ComponentFilter<CollisionComponent, const TransformComponent>,
		RaccoonEcs::ComponentFilter<MovementComponent>,
		RaccoonEcs::ComponentFilter<const CollisionComponent, const TransformComponent, MovementComponent>>(
		RaccoonEcs::SystemDependencies().goesAfter<TestCircularUnitsSystem>(),
		getWorldHolder()
	);

	getSystemsManager().registerSystem<CameraSystem,
		RaccoonEcs::ComponentFilter<const TransformComponent, MovementComponent>,
		RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent>,
		RaccoonEcs::ComponentFilter<const TransformComponent>,
		RaccoonEcs::ComponentFilter<const ImguiComponent>,
		RaccoonEcs::ComponentAdder<WorldCachedDataComponent>>(
		RaccoonEcs::SystemDependencies().goesAfter<CollisionSystem>(),
		getWorldHolder(),
		getInputData()
	);

	getSystemsManager().registerSystem<MovementSystem,
		RaccoonEcs::ComponentFilter<MovementComponent, TransformComponent>,
		RaccoonEcs::ComponentFilter<SpatialTrackComponent>,
		RaccoonEcs::ComponentFilter<TrackedSpatialEntitiesComponent>,
		RaccoonEcs::EntityTransferer>(
		RaccoonEcs::SystemDependencies().goesAfter<CollisionSystem>(),
		getWorldHolder(),
		getTime()
	);

	getSystemsManager().registerSystem<CharacterStateSystem,
		RaccoonEcs::ComponentFilter<const StateMachineComponent>,
		RaccoonEcs::ComponentFilter<CharacterStateComponent>,
		RaccoonEcs::ComponentFilter<const CharacterStateComponent, MovementComponent>,
		RaccoonEcs::ComponentFilter<const CharacterStateComponent, const MovementComponent, AnimationGroupsComponent>>(
		RaccoonEcs::SystemDependencies().goesAfter<MovementSystem>(),
		getWorldHolder(),
		getTime()
	);

	getSystemsManager().registerSystem<ResourceStreamingSystem,
		RaccoonEcs::ComponentAdder<WorldCachedDataComponent>,
		RaccoonEcs::ComponentRemover<SpriteCreatorComponent>,
		RaccoonEcs::ComponentFilter<SpriteCreatorComponent>,
		RaccoonEcs::ComponentAdder<SpriteRenderComponent>,
		RaccoonEcs::ComponentAdder<AnimationClipsComponent>,
		RaccoonEcs::ComponentRemover<AnimationClipCreatorComponent>,
		RaccoonEcs::ComponentFilter<AnimationClipCreatorComponent>,
		RaccoonEcs::ComponentAdder<AnimationGroupsComponent>,
		RaccoonEcs::ComponentRemover<AnimationGroupCreatorComponent>,
		RaccoonEcs::ComponentFilter<AnimationGroupCreatorComponent>,
		RaccoonEcs::ScheduledActionsExecutor>(
		RaccoonEcs::SystemDependencies().goesBefore<RenderSystem>(),
		getWorldHolder(),
		getResourceManager()
	);

	getSystemsManager().registerSystem<RenderSystem,
		RaccoonEcs::ComponentFilter<const WorldCachedDataComponent>,
		RaccoonEcs::ComponentFilter<const RenderModeComponent>,
		RaccoonEcs::ComponentFilter<BackgroundTextureComponent>,
		RaccoonEcs::ComponentFilter<const LightBlockingGeometryComponent>,
		RaccoonEcs::ComponentFilter<const SpriteRenderComponent, const TransformComponent>,
		RaccoonEcs::ComponentFilter<LightComponent, const TransformComponent>,
		RaccoonEcs::ComponentFilter<RenderAccessorComponent>>(
		RaccoonEcs::SystemDependencies().goesAfter<MovementSystem>(),
		getWorldHolder(),
		getTime(),
		getResourceManager(),
		getThreadPool()
	);

	startGame(
		arguments,
		[this](const RaccoonEcs::InnerDataAccessor& dataAccessor)
		{
			Vector2D playerPos{ZERO_VECTOR};

			AsyncEntityView playerEntity = getWorldHolder().getWorld().createTrackedSpatialEntity(
				RaccoonEcs::ComponentAdder<class TrackedSpatialEntitiesComponent>(dataAccessor),
				RaccoonEcs::ComponentAdder<class SpatialTrackComponent>(dataAccessor),
				RaccoonEcs::EntityAdder(dataAccessor),
				STR_TO_ID("ControlledEntity"),
				SpatialWorldData::GetCellForPos(playerPos)
			);

			{
				TransformComponent* transform = playerEntity.addComponent(RaccoonEcs::ComponentAdder<TransformComponent>(dataAccessor));
				transform->setLocation(playerPos);
			}
			{
				SpriteCreatorComponent* sprite = playerEntity.addComponent(RaccoonEcs::ComponentAdder<SpriteCreatorComponent>(dataAccessor));
				SpriteDescription spriteDesc;
				spriteDesc.params.size = Vector2D(30.0f, 30.0f);
				spriteDesc.path = "resources/textures/hero.png";
				sprite->getDescriptionsRef().emplace_back(std::move(spriteDesc));
			}
			{
				CollisionComponent* collision = playerEntity.addComponent(RaccoonEcs::ComponentAdder<CollisionComponent>(dataAccessor));
				Hull& hull = collision->getGeometryRef();
				hull.type = HullType::Circular;
				hull.setRadius(15.0f);
			}
			playerEntity.addComponent(RaccoonEcs::ComponentAdder<MovementComponent>(dataAccessor));

			Vector2D cameraPos{ZERO_VECTOR};
			AsyncEntityView camera = getWorldHolder().getWorld().createTrackedSpatialEntity(
				RaccoonEcs::ComponentAdder<class TrackedSpatialEntitiesComponent>(dataAccessor),
				RaccoonEcs::ComponentAdder<class SpatialTrackComponent>(dataAccessor),
				RaccoonEcs::EntityAdder(dataAccessor),
				STR_TO_ID("CameraEntity"), SpatialWorldData::GetCellForPos(cameraPos)
			);

			{
				TransformComponent* transform = camera.addComponent(RaccoonEcs::ComponentAdder<TransformComponent>(dataAccessor));
				transform->setLocation(cameraPos);
			}
			camera.addComponent(RaccoonEcs::ComponentAdder<MovementComponent>(dataAccessor));
		}
	);
}

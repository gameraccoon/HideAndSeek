#include "Base/precomp.h"

#include "GameLogic/Systems/AnimationSystem.h"

#include <algorithm>

#include "GameData/Components/AnimationClipsComponent.generated.h"
#include "GameData/Components/AnimationGroupsComponent.generated.h"
#include "GameData/Components/SpriteRenderComponent.generated.h"
#include "GameData/Components/StateMachineComponent.generated.h"
#include "GameData/Components/TimeComponent.generated.h"
#include "GameData/Components/WorldCachedDataComponent.generated.h"
#include "GameData/GameData.h"
#include "GameData/World.h"

#include "GameLogic/SharedManagers/WorldHolder.h"

AnimationSystem::AnimationSystem(WorldHolder& worldHolder) noexcept
	: mWorldHolder(worldHolder)
{
}

void AnimationSystem::update()
{
	SCOPED_PROFILER("AnimationSystem::update");
	World& world = mWorldHolder.getWorld();
	GameData& gameData = mWorldHolder.getGameData();

	const auto [time] = world.getWorldComponents().getComponents<const TimeComponent>();
	float dt = time->getValue().lastFixedUpdateDt;

	const auto [stateMachines] = gameData.getGameComponents().getComponents<const StateMachineComponent>();

	const auto [worldCachedData] = world.getWorldComponents().getComponents<const WorldCachedDataComponent>();
	Vector2D workingRect = worldCachedData->getScreenSize();
	Vector2D cameraLocation = worldCachedData->getCameraPos();

	SpatialEntityManager spatialManager = world.getSpatialData().getCellManagersAround(cameraLocation, workingRect);

	// update animation clip from FSM
	spatialManager.forEachComponentSet<AnimationGroupsComponent, AnimationClipsComponent>(
		[stateMachines](AnimationGroupsComponent* animationGroups, AnimationClipsComponent* animationClips)
	{
		for (auto& data : animationGroups->getDataRef())
		{
			auto smIt = stateMachines->getAnimationSMs().find(data.stateMachineId);
			Assert(smIt != stateMachines->getAnimationSMs().end(), "State machine not found %s", data.stateMachineId);
			auto newState = smIt->second.getNextState(animationGroups->getBlackboard(), data.currentState);
			if (newState != data.currentState)
			{
				data.currentState = newState;
				animationClips->getDatasRef()[data.animationClipIdx].sprites = data.animationClips.find(newState)->second;
			}
		}
	});

	// update animation frame
	spatialManager.forEachComponentSet<AnimationClipsComponent, SpriteRenderComponent>(
			[dt](AnimationClipsComponent* animationClips, SpriteRenderComponent* spriteRender)
	{
		std::vector<AnimationClip>& animationDatas = animationClips->getDatasRef();
		for (auto& data : animationDatas)
		{
			data.progress += data.params.speed * dt;
			if (data.progress >= 1.0f && data.params.isLooped)
			{
				data.progress -= 1.0f;
			}

			size_t spriteIdx = 0;

			const auto& ids = spriteRender->getSpriteIds();
			for (size_t i = 0; i < spriteRender->getSpriteIds().size(); ++i)
			{
				if (ids[i] == data.spriteId)
				{
					spriteIdx = i;
				}
			}

			spriteRender->getSpriteDatasRef()[spriteIdx].spriteHandle = data.sprites[std::min(static_cast<size_t>(data.sprites.size() * data.progress), data.sprites.size())];
		}
	});
}

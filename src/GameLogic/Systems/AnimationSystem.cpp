#include "Base/precomp.h"

#include "GameLogic/Systems/AnimationSystem.h"

#include <algorithm>

#include "GameData/Components/RenderComponent.generated.h"
#include "GameData/Components/AnimationClipsComponent.generated.h"
#include "GameData/Components/AnimationGroupsComponent.generated.h"
#include "GameData/Components/StateMachineComponent.generated.h"
#include "GameData/Components/WorldCachedDataComponent.generated.h"
#include "GameData/World.h"
#include "GameData/GameData.h"

AnimationSystem::AnimationSystem(
		RaccoonEcs::ComponentFilter<AnimationGroupsComponent, AnimationClipsComponent>&& animUpdateFilter,
		RaccoonEcs::ComponentFilter<AnimationClipsComponent, RenderComponent>&& animRenderFilter,
		RaccoonEcs::ComponentFilter<const StateMachineComponent>&& stateMachineFilter,
		RaccoonEcs::ComponentFilter<const WorldCachedDataComponent>&& worldCachedDataFilter,
		WorldHolder& worldHolder,
		const TimeData& time) noexcept
	: mAnimUpdateFilter(std::move(animUpdateFilter))
	, mAnimRenderFilter(std::move(animRenderFilter))
	, mStateMachineFilter(std::move(stateMachineFilter))
	, mWorldCachedDataFilter(std::move(worldCachedDataFilter))
	, mWorldHolder(worldHolder)
	, mTime(time)
{
}

void AnimationSystem::update()
{
	World& world = mWorldHolder.getWorld();
	GameData& gameData = mWorldHolder.getGameData();
	float dt = mTime.dt;

	const auto [stateMachines] = mStateMachineFilter.getComponents(gameData.getGameComponents());

	const auto [worldCachedData] = mWorldCachedDataFilter.getComponents(world.getWorldComponents());
	Vector2D workingRect = worldCachedData->getScreenSize();
	Vector2D cameraLocation = worldCachedData->getCameraPos();

	SpatialEntityManager spatialManager = world.getSpatialData().getCellManagersAround(cameraLocation, workingRect);

	// update animation clip from FSM
	spatialManager.forEachComponentSet(
		mAnimUpdateFilter,
		[dt, stateMachines](AnimationGroupsComponent* animationGroups, AnimationClipsComponent* animationClips)
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
	spatialManager.forEachComponentSet(
			mAnimRenderFilter,
			[dt](AnimationClipsComponent* animationClips, RenderComponent* render)
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

			const auto& ids = render->getSpriteIds();
			for (size_t i = 0; i < render->getSpriteIds().size(); ++i)
			{
				if (ids[i] == data.spriteId)
				{
					spriteIdx = i;
				}
			}

			render->getSpriteDatasRef()[spriteIdx].spriteHandle = data.sprites[std::min(static_cast<size_t>(data.sprites.size() * data.progress), data.sprites.size())];
		}
	});
}

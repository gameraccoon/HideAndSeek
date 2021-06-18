#pragma once

#include <memory>

#include <raccoon-ecs/system.h>
#include <raccoon-ecs/async_operations.h>

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"

#include "GameData/Components/RenderComponent.generated.h"
#include "GameData/Components/AnimationClipsComponent.generated.h"
#include "GameData/Components/AnimationGroupsComponent.generated.h"
#include "GameData/Components/StateMachineComponent.generated.h"
#include "GameData/Components/WorldCachedDataComponent.generated.h"

/**
 * System that updates animations
 */
class AnimationSystem : public RaccoonEcs::System
{
public:
	AnimationSystem(
		RaccoonEcs::ComponentFilter<AnimationGroupsComponent, AnimationClipsComponent>&& animUpdateFilter,
		RaccoonEcs::ComponentFilter<AnimationClipsComponent, RenderComponent>&& animRenderFilter,
		RaccoonEcs::ComponentFilter<const StateMachineComponent>&& stateMachineFilter,
		RaccoonEcs::ComponentFilter<const WorldCachedDataComponent>&& worldCachedDataFilter,
		WorldHolder& worldHolder,
		const TimeData& timeData) noexcept;
	~AnimationSystem() override = default;

	void update() override;
	static std::string GetSystemId() { return "AnimationSystem"; }

private:
	RaccoonEcs::ComponentFilter<AnimationGroupsComponent, AnimationClipsComponent> mAnimUpdateFilter;
	RaccoonEcs::ComponentFilter<AnimationClipsComponent, RenderComponent> mAnimRenderFilter;
	RaccoonEcs::ComponentFilter<const StateMachineComponent> mStateMachineFilter;
	RaccoonEcs::ComponentFilter<const WorldCachedDataComponent> mWorldCachedDataFilter;
	WorldHolder& mWorldHolder;
	const TimeData& mTime;
};

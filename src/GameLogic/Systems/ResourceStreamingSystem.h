#pragma once

#include <memory>

#include <raccoon-ecs/system.h>
#include <raccoon-ecs/async_operations.h>

#include "GameData/Components/SpriteCreatorComponent.generated.h"
#include "GameData/Components/SpriteRenderComponent.generated.h"
#include "GameData/Components/AnimationClipsComponent.generated.h"
#include "GameData/Components/AnimationClipCreatorComponent.generated.h"
#include "GameData/Components/AnimationGroupsComponent.generated.h"
#include "GameData/Components/AnimationGroupCreatorComponent.generated.h"
#include "GameData/Components/WorldCachedDataComponent.generated.h"

#include "HAL/Base/ResourceManager.h"

#include "GameLogic/SharedManagers/WorldHolder.h"

/**
 * System that loads and distributes resources
 */
class ResourceStreamingSystem : public RaccoonEcs::System
{
public:
	ResourceStreamingSystem(
		RaccoonEcs::ComponentFilter<WorldCachedDataComponent>&& worldCachedDataFilter,
		RaccoonEcs::ComponentRemover<SpriteCreatorComponent>&& spriteCreatorRemover,
		RaccoonEcs::ComponentFilter<SpriteCreatorComponent>&& spriteCreatorFilter,
		RaccoonEcs::ComponentAdder<SpriteRenderComponent>&& spriteRenderComponentAdder,
		RaccoonEcs::ComponentAdder<AnimationClipsComponent>&& animationClipsAdder,
		RaccoonEcs::ComponentRemover<AnimationClipCreatorComponent>&& animationClipCreatorRemover,
		RaccoonEcs::ComponentFilter<AnimationClipCreatorComponent>&& animationClipCreatorFilter,
		RaccoonEcs::ComponentAdder<AnimationGroupsComponent>&& animationGroupsAdder,
		RaccoonEcs::ComponentRemover<AnimationGroupCreatorComponent>&& animationGroupCreatorRemover,
		RaccoonEcs::ComponentFilter<AnimationGroupCreatorComponent>&& animationGroupCreatorFilter,
		RaccoonEcs::ScheduledActionsExecutor&& scheduledActionsExecutor,
		WorldHolder& worldHolder,
		HAL::ResourceManager& resourceManager) noexcept;
	~ResourceStreamingSystem() override = default;

	void update() override;
	static std::string GetSystemId() { return "ResourceStreamingSystem"; }

private:
	RaccoonEcs::ComponentFilter<WorldCachedDataComponent> mWorldCachedDataFilter;
	RaccoonEcs::ComponentRemover<SpriteCreatorComponent> mSpriteCreatorRemover;
	RaccoonEcs::ComponentFilter<SpriteCreatorComponent> mSpriteCreatorFilter;
	RaccoonEcs::ComponentAdder<SpriteRenderComponent> mSpriteRenderComponentAdder;
	RaccoonEcs::ComponentAdder<AnimationClipsComponent> mAnimationClipsAdder;
	RaccoonEcs::ComponentRemover<AnimationClipCreatorComponent> mAnimationClipCreatorRemover;
	RaccoonEcs::ComponentFilter<AnimationClipCreatorComponent> mAnimationClipCreatorFilter;
	RaccoonEcs::ComponentAdder<AnimationGroupsComponent> mAnimationGroupsAdder;
	RaccoonEcs::ComponentRemover<AnimationGroupCreatorComponent> mAnimationGroupCreatorRemover;
	RaccoonEcs::ComponentFilter<AnimationGroupCreatorComponent> mAnimationGroupCreatorFilter;
	RaccoonEcs::ScheduledActionsExecutor mScheduledActionsExecutor;
	WorldHolder& mWorldHolder;
	HAL::ResourceManager& mResourceManager;
};

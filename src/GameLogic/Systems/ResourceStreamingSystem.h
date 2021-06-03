#pragma once

#include <memory>

#include <raccoon-ecs/system.h>
#include <raccoon-ecs/async_operations.h>

#include "HAL/Base/ResourceManager.h"

#include "GameLogic/SharedManagers/WorldHolder.h"

class WorldCachedDataComponent;
class SpriteCreatorComponent;
class RenderComponent;
class AnimationClipsComponent;
class AnimationClipCreatorComponent;
class AnimationGroupsComponent;
class AnimationGroupCreatorComponent;

/**
 * System that loads and distributes resources
 */
class ResourceStreamingSystem : public RaccoonEcs::System
{
public:
	ResourceStreamingSystem(
		RaccoonEcs::ComponentAdder<WorldCachedDataComponent>&& worldCachedDataAdder,
		RaccoonEcs::ComponentRemover<SpriteCreatorComponent>&& spriteCreatorRemover,
		RaccoonEcs::ComponentFilter<SpriteCreatorComponent>&& spriteCreatorFilter,
		RaccoonEcs::ComponentAdder<RenderComponent>&& renderComponentAdder,
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
	std::string getName() const override { return "ResourceStreamingSystem"; }

private:
	RaccoonEcs::ComponentAdder<WorldCachedDataComponent> mWorldCachedDataAdder;
	RaccoonEcs::ComponentRemover<SpriteCreatorComponent> mSpriteCreatorRemover;
	RaccoonEcs::ComponentFilter<SpriteCreatorComponent> mSpriteCreatorFilter;
	RaccoonEcs::ComponentAdder<RenderComponent> mRenderComponentAdder;
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

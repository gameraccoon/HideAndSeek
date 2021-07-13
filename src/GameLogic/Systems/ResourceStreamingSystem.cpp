#include "Base/precomp.h"

#include "GameLogic/Systems/ResourceStreamingSystem.h"

#include "GameData/World.h"
#include "GameData/GameData.h"

#include "HAL/Graphics/Sprite.h"
#include "HAL/Graphics/SpriteAnimationClip.h"
#include "HAL/Graphics/AnimationGroup.h"

ResourceStreamingSystem::ResourceStreamingSystem(
		RaccoonEcs::ComponentAdder<WorldCachedDataComponent>&& worldCachedDataAddeer,
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
		HAL::ResourceManager& resourceManager) noexcept
	: mWorldCachedDataAdder(std::move(worldCachedDataAddeer))
	, mSpriteCreatorRemover(std::move(spriteCreatorRemover))
	, mSpriteCreatorFilter(std::move(spriteCreatorFilter))
	, mSpriteRenderComponentAdder(std::move(spriteRenderComponentAdder))
	, mAnimationClipsAdder(std::move(animationClipsAdder))
	, mAnimationClipCreatorRemover(std::move(animationClipCreatorRemover))
	, mAnimationClipCreatorFilter(std::move(animationClipCreatorFilter))
	, mAnimationGroupsAdder(std::move(animationGroupsAdder))
	, mAnimationGroupCreatorRemover(std::move(animationGroupCreatorRemover))
	, mAnimationGroupCreatorFilter(std::move(animationGroupCreatorFilter))
	, mScheduledActionsExecutor(std::move(scheduledActionsExecutor))
	, mWorldHolder(worldHolder)
	, mResourceManager(resourceManager)
{
}

void ResourceStreamingSystem::update()
{
	World& world = mWorldHolder.getWorld();

	WorldCachedDataComponent* worldCachedData = mWorldCachedDataAdder.getOrAddComponent(world.getWorldComponents());
	Vector2D workingRect = worldCachedData->getScreenSize();

	// load sprites
	SpatialEntityManager spatialManager = world.getSpatialData().getCellManagersAround(worldCachedData->getCameraPos(), workingRect);
	spatialManager.forEachSpatialComponentSetWithEntity(
			mSpriteCreatorFilter,
			[this](WorldCell* cell, Entity entity, SpriteCreatorComponent* spriteCreator)
	{
		AsyncEntityView entityView{entity, cell->getEntityManager() };
		const auto& descriptions = spriteCreator->getDescriptions();
		Assert(!descriptions.empty(), "Sprite descriptions should not be empty");

		SpriteRenderComponent* spriteRender = entityView.scheduleAddComponent(mSpriteRenderComponentAdder);
		size_t spritesCount = descriptions.size();
		auto& spriteDatas = spriteRender->getSpriteDatasRef();
		spriteDatas.resize(spritesCount);
		for (size_t i = 0; i < spritesCount; ++i)
		{
			spriteDatas[i].spriteHandle = mResourceManager.lockSprite(descriptions[i].path);
			spriteDatas[i].params = descriptions[i].params;
			int id = spriteRender->getMaxSpriteId();
			spriteRender->getSpriteIdsRef().push_back(id++);
			spriteRender->setMaxSpriteId(id);
		}
		entityView.scheduleRemoveComponent(mSpriteCreatorRemover);
	});
	spatialManager.executeScheduledActions(mScheduledActionsExecutor);

	// load single animations clips
	spatialManager.forEachSpatialComponentSetWithEntity(
			mAnimationClipCreatorFilter,
			[this](WorldCell* cell, Entity entity, AnimationClipCreatorComponent* animationClipCreator)
	{
		AsyncEntityView entityView{entity, cell->getEntityManager() };

		const auto& descriptions = animationClipCreator->getDescriptionsRef();
		Assert(!descriptions.empty(), "Animation descriptions should not be empty");

		AnimationClipsComponent* animationClips = entityView.scheduleAddComponent(mAnimationClipsAdder);
		size_t animationCount = descriptions.size();
		auto& animations = animationClips->getDatasRef();
		animations.resize(animationCount);

		auto [spriteRender] = entityView.getComponents(mSpriteRenderComponentAdder);
		if (spriteRender == nullptr)
		{
			spriteRender = entityView.scheduleAddComponent(mSpriteRenderComponentAdder);
		}

		auto& spriteDatas = spriteRender->getSpriteDatasRef();
		for (size_t i = 0; i < animationCount; ++i)
		{
			animations[i].animation = mResourceManager.lockSpriteAnimationClip(descriptions[i].path);
			animations[i].params = descriptions[i].params;
			animations[i].sprites = mResourceManager.getResource<Graphics::SpriteAnimationClip>(animations[i].animation).getSprites();

			AssertFatal(!animations[i].sprites.empty(), "Empty SpriteAnimation '%s'", descriptions[i].path.c_str());
			spriteDatas.emplace_back(descriptions[i].spriteParams, animations[i].sprites.front());
			int id = spriteRender->getMaxSpriteId();
			animations[i].spriteId = id;
			spriteRender->getSpriteIdsRef().push_back(id++);
			spriteRender->setMaxSpriteId(id);

			Assert(spriteRender->getSpriteIds().size() == spriteDatas.size(), "Sprites and SpriteIds have diverged");
		}

		entityView.scheduleAddComponent(mAnimationClipsAdder);

		entityView.scheduleRemoveComponent(mAnimationClipCreatorRemover);
	});
	spatialManager.executeScheduledActions(mScheduledActionsExecutor);

	// load animation groups
	spatialManager.forEachSpatialComponentSetWithEntity(
			mAnimationGroupCreatorFilter,
			[this](WorldCell* cell, Entity entity, AnimationGroupCreatorComponent* animationGroupCreator)
	{
		AsyncEntityView entityView{entity, cell->getEntityManager() };
		AnimationGroupsComponent* animationGroups = entityView.scheduleAddComponent(mAnimationGroupsAdder);

		auto [animationClips] = entityView.getComponents(mAnimationClipsAdder);
		if (animationClips == nullptr)
		{
			animationClips = entityView.scheduleAddComponent(mAnimationClipsAdder);
		}
		auto& clipDatas = animationClips->getDatasRef();

		auto [spriteRender] = entityView.getComponents(mSpriteRenderComponentAdder);
		if (spriteRender == nullptr)
		{
			spriteRender = entityView.scheduleAddComponent(mSpriteRenderComponentAdder);
		}
		auto& spriteDatas = spriteRender->getSpriteDatasRef();

		size_t i = 0;
		for (const ResourcePath& groupPath : animationGroupCreator->getAnimationGroups())
		{
			ResourceHandle animGroupHandle = mResourceManager.lockAnimationGroup(groupPath);
			const Graphics::AnimationGroup& group = mResourceManager.getResource<Graphics::AnimationGroup>(animGroupHandle);
			AnimationGroup<StringId> animationGroup;
			animationGroup.currentState = group.getDefaultState();
			animationGroup.animationClips = group.getAnimationClips();
			animationGroup.stateMachineId = group.getStateMachineId();
			animationGroup.animationClipIdx = clipDatas.size();

			AnimationClip clip;
			clip.params = animationGroupCreator->getClipParams()[i];
			clip.sprites = animationGroup.animationClips.find(animationGroup.currentState)->second;

			animationGroups->getDataRef().emplace_back(std::move(animationGroup));

			spriteDatas.emplace_back(animationGroupCreator->getSpriteParams()[i], clip.sprites.front());

			int id = spriteRender->getMaxSpriteId();
			clip.spriteId = id;
			spriteRender->getSpriteIdsRef().push_back(id++);
			spriteRender->setMaxSpriteId(id);

			clipDatas.emplace_back(std::move(clip));

			++i;
		}

		entityView.scheduleRemoveComponent(mAnimationGroupCreatorRemover);
	});
	spatialManager.executeScheduledActions(mScheduledActionsExecutor);
}

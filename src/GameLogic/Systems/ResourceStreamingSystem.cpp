#include "Base/precomp.h"

#include "GameLogic/Systems/ResourceStreamingSystem.h"

#include "GameData/Components/SpriteCreatorComponent.generated.h"
#include "GameData/Components/RenderComponent.generated.h"
#include "GameData/Components/AnimationClipsComponent.generated.h"
#include "GameData/Components/AnimationClipCreatorComponent.generated.h"
#include "GameData/Components/AnimationGroupsComponent.generated.h"
#include "GameData/Components/AnimationGroupCreatorComponent.generated.h"
#include "GameData/Components/WorldCachedDataComponent.generated.h"

#include "GameData/World.h"
#include "GameData/GameData.h"

#include "HAL/Graphics/Sprite.h"
#include "HAL/Graphics/SpriteAnimationClip.h"
#include "HAL/Graphics/AnimationGroup.h"

ResourceStreamingSystem::ResourceStreamingSystem(
		RaccoonEcs::ComponentAdder<WorldCachedDataComponent>&& worldCachedDataAddeer,
		RaccoonEcs::ComponentRemover<SpriteCreatorComponent>&& spriteCreatorRemover,
		RaccoonEcs::ComponentFilter<SpriteCreatorComponent>&& spriteCreatorFilter,
		RaccoonEcs::ComponentAdder<RenderComponent>&& renderComponentAdder,
		RaccoonEcs::ComponentAdder<AnimationClipsComponent>&& animationClipsAdder,
		RaccoonEcs::ComponentRemover<AnimationClipCreatorComponent>&& animationClipCreatorRemover,
		RaccoonEcs::ComponentFilter<AnimationClipCreatorComponent>&& animationClipCreatorFilter,
		RaccoonEcs::ComponentAdder<AnimationGroupsComponent>&& animationGroupsAdder,
		RaccoonEcs::ComponentRemover<AnimationGroupCreatorComponent>&& animationGroupCreatorRemover,
		RaccoonEcs::ComponentFilter<AnimationGroupCreatorComponent>&& animationGroupCreatorFilter,
		WorldHolder& worldHolder,
		HAL::ResourceManager& resourceManager) noexcept
	: mWorldCachedDataAdder(std::move(worldCachedDataAddeer))
	, mSpriteCreatorRemover(std::move(spriteCreatorRemover))
	, mSpriteCreatorFilter(std::move(spriteCreatorFilter))
	, mRenderComponentAdder(std::move(renderComponentAdder))
	, mAnimationClipsAdder(std::move(animationClipsAdder))
	, mAnimationClipCreatorRemover(std::move(animationClipCreatorRemover))
	, mAnimationClipCreatorFilter(std::move(animationClipCreatorFilter))
	, mAnimationGroupsAdder(std::move(animationGroupsAdder))
	, mAnimationGroupCreatorRemover(std::move(animationGroupCreatorRemover))
	, mAnimationGroupCreatorFilter(std::move(animationGroupCreatorFilter))
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
	spatialManager.forEachSpatialComponentSetWithEntityN(
			mSpriteCreatorFilter,
			[this](WorldCell* cell, Entity entity, SpriteCreatorComponent* spriteCreator)
	{
		EntityView entityView{entity, cell->getEntityManager() };
		const auto& descriptions = spriteCreator->getDescriptions();
		Assert(!descriptions.empty(), "Sprite descriptions should not be empty");

		RenderComponent* render = mRenderComponentAdder.scheduleAddComponent(entityView);
		size_t spritesCount = descriptions.size();
		auto& spriteDatas = render->getSpriteDatasRef();
		spriteDatas.resize(spritesCount);
		for (size_t i = 0; i < spritesCount; ++i)
		{
			spriteDatas[i].spriteHandle = mResourceManager.lockSprite(descriptions[i].path);
			spriteDatas[i].params = descriptions[i].params;
			int id = render->getMaxSpriteId();
			render->getSpriteIdsRef().push_back(id++);
			render->setMaxSpriteId(id);
		}
		mSpriteCreatorRemover.scheduleRemoveComponent(entityView);
	});
	spatialManager.executeScheduledActions();

	// load single animations clips
	spatialManager.forEachSpatialComponentSetWithEntityN(
			mAnimationClipCreatorFilter,
			[this](WorldCell* cell, Entity entity, AnimationClipCreatorComponent* animationClipCreator)
	{
		EntityView entityView{entity, cell->getEntityManager() };
		const auto& descriptions = animationClipCreator->getDescriptionsRef();
		Assert(!descriptions.empty(), "Animation descriptions should not be empty");

		AnimationClipsComponent* animationClips = mAnimationClipsAdder.scheduleAddComponent(entityView);
		size_t animationCount = descriptions.size();
		auto& animations = animationClips->getDatasRef();
		animations.resize(animationCount);

		auto [render] = mRenderComponentAdder.getComponents(entityView);
		if (render == nullptr)
		{
			render = mRenderComponentAdder.scheduleAddComponent(entityView);
		}

		auto& spriteDatas = render->getSpriteDatasRef();
		for (size_t i = 0; i < animationCount; ++i)
		{
			animations[i].animation = mResourceManager.lockSpriteAnimationClip(descriptions[i].path);
			animations[i].params = descriptions[i].params;
			animations[i].sprites = mResourceManager.getResource<Graphics::SpriteAnimationClip>(animations[i].animation).getSprites();

			AssertFatal(!animations[i].sprites.empty(), "Empty SpriteAnimation '%s'", descriptions[i].path.c_str());
			spriteDatas.emplace_back(descriptions[i].spriteParams, animations[i].sprites.front());
			int id = render->getMaxSpriteId();
			animations[i].spriteId = id;
			render->getSpriteIdsRef().push_back(id++);
			render->setMaxSpriteId(id);

			Assert(render->getSpriteIds().size() == spriteDatas.size(), "Sprites and SpriteIds have diverged");
		}

		mAnimationClipsAdder.scheduleAddComponent(entityView);

		mAnimationClipCreatorRemover.scheduleRemoveComponent(entityView);
	});
	spatialManager.executeScheduledActions();

	// load animation groups
	spatialManager.forEachSpatialComponentSetWithEntityN(
			mAnimationGroupCreatorFilter,
			[this](WorldCell* cell, Entity entity, AnimationGroupCreatorComponent* animationGroupCreator)
	{
		EntityView entityView{entity, cell->getEntityManager() };
		AnimationGroupsComponent* animationGroups = mAnimationGroupsAdder.scheduleAddComponent(entityView);

		auto [animationClips] = mAnimationClipsAdder.getComponents(entityView);
		if (animationClips == nullptr)
		{
			animationClips = mAnimationClipsAdder.scheduleAddComponent(entityView);
		}
		auto& clipDatas = animationClips->getDatasRef();

		auto [render] = mRenderComponentAdder.getComponents(entityView);
		if (render == nullptr)
		{
			render = mRenderComponentAdder.scheduleAddComponent(entityView);
		}
		auto& spriteDatas = render->getSpriteDatasRef();

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

			int id = render->getMaxSpriteId();
			clip.spriteId = id;
			render->getSpriteIdsRef().push_back(id++);
			render->setMaxSpriteId(id);

			clipDatas.emplace_back(std::move(clip));

			++i;
		}

		mAnimationGroupCreatorRemover.scheduleRemoveComponent(entityView);
	});
	spatialManager.executeScheduledActions();
}

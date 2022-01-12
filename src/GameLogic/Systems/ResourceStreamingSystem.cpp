#include "Base/precomp.h"

#include "GameLogic/Systems/ResourceStreamingSystem.h"

#include "GameData/World.h"
#include "GameData/GameData.h"
#include "GameData/Components/SpriteCreatorComponent.generated.h"
#include "GameData/Components/SpriteRenderComponent.generated.h"
#include "GameData/Components/AnimationClipsComponent.generated.h"
#include "GameData/Components/AnimationClipCreatorComponent.generated.h"
#include "GameData/Components/AnimationGroupsComponent.generated.h"
#include "GameData/Components/AnimationGroupCreatorComponent.generated.h"
#include "GameData/Components/WorldCachedDataComponent.generated.h"

#include "HAL/Graphics/Sprite.h"
#include "HAL/Graphics/SpriteAnimationClip.h"
#include "HAL/Graphics/AnimationGroup.h"

ResourceStreamingSystem::ResourceStreamingSystem(
		WorldHolder& worldHolder,
		HAL::ResourceManager& resourceManager) noexcept
	: mWorldHolder(worldHolder)
	, mResourceManager(resourceManager)
{
}

void ResourceStreamingSystem::update()
{
	SCOPED_PROFILER("ResourceStreamingSystem::update");
	World& world = mWorldHolder.getWorld();

	auto [worldCachedData] = world.getWorldComponents().getComponents<const WorldCachedDataComponent>();
	Vector2D workingRect = worldCachedData->getScreenSize();

	// load sprites
	SpatialEntityManager spatialManager = world.getSpatialData().getCellManagersAround(worldCachedData->getCameraPos(), workingRect);
	spatialManager.forEachSpatialComponentSetWithEntity<SpriteCreatorComponent>(
			[this](WorldCell* cell, Entity entity, SpriteCreatorComponent* spriteCreator)
	{
		EntityView entityView{entity, cell->getEntityManager() };
		const auto& descriptions = spriteCreator->getDescriptions();
		Assert(!descriptions.empty(), "Sprite descriptions should not be empty");

		SpriteRenderComponent* spriteRender = entityView.scheduleAddComponent<SpriteRenderComponent>();
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
		entityView.scheduleRemoveComponent<SpriteCreatorComponent>();
	});
	spatialManager.executeScheduledActions();

	// load single animations clips
	spatialManager.forEachSpatialComponentSetWithEntity<AnimationClipCreatorComponent>(
			[this](WorldCell* cell, Entity entity, AnimationClipCreatorComponent* animationClipCreator)
	{
		EntityView entityView{entity, cell->getEntityManager() };

		const auto& descriptions = animationClipCreator->getDescriptionsRef();
		Assert(!descriptions.empty(), "Animation descriptions should not be empty");

		AnimationClipsComponent* animationClips = entityView.scheduleAddComponent<AnimationClipsComponent>();
		size_t animationCount = descriptions.size();
		auto& animations = animationClips->getDatasRef();
		animations.resize(animationCount);

		auto [spriteRender] = entityView.getComponents<SpriteRenderComponent>();
		if (spriteRender == nullptr)
		{
			spriteRender = entityView.scheduleAddComponent<SpriteRenderComponent>();
		}

		auto& spriteDatas = spriteRender->getSpriteDatasRef();
		for (size_t i = 0; i < animationCount; ++i)
		{
			animations[i].animation = mResourceManager.lockSpriteAnimationClip(descriptions[i].path);
			animations[i].params = descriptions[i].params;
			animations[i].sprites = mResourceManager.tryGetResource<Graphics::SpriteAnimationClip>(animations[i].animation)->getSprites();

			AssertFatal(!animations[i].sprites.empty(), "Empty SpriteAnimation '%s'", descriptions[i].path.c_str());
			spriteDatas.emplace_back(descriptions[i].spriteParams, animations[i].sprites.front());
			int id = spriteRender->getMaxSpriteId();
			animations[i].spriteId = id;
			spriteRender->getSpriteIdsRef().push_back(id++);
			spriteRender->setMaxSpriteId(id);

			Assert(spriteRender->getSpriteIds().size() == spriteDatas.size(), "Sprites and SpriteIds have diverged");
		}

		entityView.scheduleAddComponent<AnimationClipsComponent>();

		entityView.scheduleRemoveComponent<AnimationClipCreatorComponent>();
	});
	spatialManager.executeScheduledActions();

	// load animation groups
	spatialManager.forEachSpatialComponentSetWithEntity<AnimationGroupCreatorComponent>(
			[this](WorldCell* cell, Entity entity, AnimationGroupCreatorComponent* animationGroupCreator)
	{
		EntityView entityView{entity, cell->getEntityManager() };
		AnimationGroupsComponent* animationGroups = entityView.scheduleAddComponent<AnimationGroupsComponent>();

		auto [animationClips] = entityView.getComponents<AnimationClipsComponent>();
		if (animationClips == nullptr)
		{
			animationClips = entityView.scheduleAddComponent<AnimationClipsComponent>();
		}
		auto& clipDatas = animationClips->getDatasRef();

		auto [spriteRender] = entityView.getComponents<SpriteRenderComponent>();
		if (spriteRender == nullptr)
		{
			spriteRender = entityView.scheduleAddComponent<SpriteRenderComponent>();
		}
		auto& spriteDatas = spriteRender->getSpriteDatasRef();

		size_t i = 0;
		for (const ResourcePath& groupPath : animationGroupCreator->getAnimationGroups())
		{
			ResourceHandle animGroupHandle = mResourceManager.lockAnimationGroup(groupPath);
			const Graphics::AnimationGroup* group = mResourceManager.tryGetResource<Graphics::AnimationGroup>(animGroupHandle);
			AnimationGroup<StringId> animationGroup;
			animationGroup.currentState = group->getDefaultState();
			animationGroup.animationClips = group->getAnimationClips();
			animationGroup.stateMachineId = group->getStateMachineId();
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

		entityView.scheduleRemoveComponent<AnimationGroupCreatorComponent>();
	});
	spatialManager.executeScheduledActions();
}

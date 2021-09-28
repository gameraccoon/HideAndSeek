#pragma once

#include <vector>

#include <GameData/Core/ResourceHandle.h>

namespace HAL
{
	/**
	 * Base class for any resource type
	 */
	class Resource
	{
	public:
		struct SpecialThreadInit
		{
			enum class Thread
			{
				Loading,
				Render
			};
			using StaticBoolFn = bool(*)(Resource*);
			using StaticVoidFn = void(*)(Resource*);

			struct Step
			{
				Thread thread;
				StaticBoolFn init;
				StaticVoidFn deinit;
			};

			SpecialThreadInit(std::initializer_list<Step> initList)
				: steps(initList)
			{}

			std::vector<Step> steps;
		};

	public:
		Resource() = default;
		virtual ~Resource() = default;

		// prohibit copying and moving so we can store raw references
		// to the resource as long as the resource is loaded
		Resource(const Resource&) = delete;
		Resource& operator=(Resource&) = delete;
		Resource(Resource&&) = delete;
		Resource& operator=(Resource&&) = delete;

		virtual bool isValid() const = 0;

		virtual const SpecialThreadInit* getSpecialThreadInitialization() const { return nullptr; };
	};
}

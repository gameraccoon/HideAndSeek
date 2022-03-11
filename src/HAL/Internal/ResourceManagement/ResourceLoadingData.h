#pragma once

#include "Base/Types/ComplexTypes/UniqueAny.h"

#include "HAL/Base/Resource.h"
#include "HAL/Internal/ResourceManagement/ResourceDependencies.h"

namespace HAL
{
	// data for loading and resolving dependencies on load
	class ResourceLoading
	{
	public:
		struct LoadingData
		{
			LoadingData(
				ResourceHandle handle,
				Resource::InitSteps&& loadingSteps,
				UniqueAny&& defaultData
			)
				: handle(handle)
				, stepsLeft(std::move(loadingSteps))
				, resourceData(std::move(defaultData))
			{}

			ResourceHandle handle;
			Resource::InitSteps stepsLeft;
			UniqueAny resourceData;
		};

		struct UnloadingData
		{
			UnloadingData(
				ResourceHandle handle,
				UniqueAny&& resourceData,
				Resource::DeinitSteps&& unloadingSteps
			)
				: handle(handle)
				, resourceData(std::move(resourceData))
				, stepsLeft(std::move(unloadingSteps))
			{}

			ResourceHandle handle;
			UniqueAny resourceData;
			Resource::DeinitSteps stepsLeft;
		};

		using LoadingDataPtr = std::unique_ptr<LoadingData>;
		using UnloadingDataPtr = std::unique_ptr<UnloadingData>;

	public:
		std::vector<LoadingDataPtr> resourcesWaitingInit;
		std::unordered_map<ResourceHandle, LoadingDataPtr> resourcesWaitingDependencies;
		std::vector<UnloadingDataPtr> resourcesWaitingDeinit;
		LoadDependencies loadDependencies;
	};
}

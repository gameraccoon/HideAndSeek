#pragma once

#include <any>
#include <optional>
#include <vector>
#include <memory>

#include "Base/Types/TemplateAliases.h"

#include "GameData/EcsDefinitions.h"
#include "GameData/Debug/SubstringSearcher.h"

#include "GameLogic/Imgui/ComponentInspector/PropertyFilters/AbstractPropertyFilter.h"

class WorldCell;
struct ImguiDebugData;

namespace ImguiPropertyFiltration
{
	class AbstractPropertyDescriptor;
	class AbstractPropertyFilterFactory;

	class ImguiPropertyFiltersWidget
	{
	public:
		~ImguiPropertyFiltersWidget();

		void update(ImguiDebugData& debugData, const RaccoonEcs::InnerDataAccessor& dataAccessor);

		void getFilteredEntities(ImguiDebugData& debugData, const RaccoonEcs::InnerDataAccessor& dataAccessor, TupleVector<WorldCell*, Entity>& inOutEntities);

	private:
		[[nodiscard]] std::vector<StringId> getFilteredComponentTypes() const;
		void init(ImguiDebugData& debugData);

	private:
		static constexpr int MinimalSearchLen = 2;

		char mFilterQueryBuffer[128] = "";
		SubstringSearcher<std::shared_ptr<AbstractPropertyDescriptor>> mPropertyDescriptors;
		std::vector<std::unique_ptr<AbstractPropertyFilter>> mAppliedFilters;
		std::vector<std::shared_ptr<AbstractPropertyFilterFactory>> mSuggestedFiltersFactories;
		std::vector<std::shared_ptr<AbstractPropertyDescriptor>> mLastMatchedProperties;
		std::optional<std::tuple<WorldCell*, Entity>> mExplicitlySetEntity;
		bool mIsInited = false;
	};
} // namespace ImguiPropertyFiltration

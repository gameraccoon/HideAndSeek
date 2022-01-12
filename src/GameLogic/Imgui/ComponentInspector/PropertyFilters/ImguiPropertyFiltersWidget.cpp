#include "Base/precomp.h"

#include "GameLogic/Imgui/ComponentInspector/PropertyFilters/ImguiPropertyFiltersWidget.h"

#include <algorithm>
#include <cstring>
#include <string>
#include <sstream>

#include "imgui.h"

#include <raccoon-ecs/entity_manager.h>
#include <raccoon-ecs/component_factory.h>

#include "GameData/Spatial/SpatialEntityManager.h"
#include "GameData/World.h"

#include "GameLogic/SharedManagers/WorldHolder.h"

#include "GameLogic/Imgui/ComponentInspector/PropertyFilters/AbstractPropertyDescriptor.h"

#include "GameLogic/Imgui/ComponentInspector/PropertyFilters/TypeFilters/FilterRegistration/FilterRegistration.h"
#include "GameLogic/Imgui/ComponentInspector/PropertyFilters/PropertyDescriptorsRegistration.h"

#include "GameLogic/Imgui/ComponentInspector/PropertyFilters/PropertyDescriptors/ComponentAvailabilityPropertyDescriptor.h"

#include "GameLogic/Imgui/ImguiDebugData.h"

namespace ImguiPropertyFiltration
{
	void ImguiPropertyFiltersWidget::init(ImguiDebugData& debugData)
	{
		auto propertyDescriptions = PropertyDescriptorsRegistration::GetDescriptions();
		debugData.componentFactory.forEachComponentType([&propertyDescriptions](StringId className)
		{
			std::string componentName = ID_TO_STR(className);
			std::string lowerComponentName = componentName;
			std::transform(lowerComponentName.begin(), lowerComponentName.end(), lowerComponentName.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
			propertyDescriptions.emplace_back(std::vector<std::string>{lowerComponentName}, ComponentAvailabilityPropertyDescriptor::Create(componentName, className));
		});
		mPropertyDescriptors.construct(std::move(propertyDescriptions));
	}

	ImguiPropertyFiltersWidget::~ImguiPropertyFiltersWidget() = default;

	static WorldCell* findEntityCell(std::unordered_map<CellPos, WorldCell>& cells, Entity entity)
	{
		for (auto& [pos, cell] : cells)
		{
			if (cell.getEntityManager().hasEntity(entity))
			{
				return &cell;
			}
		}
		return nullptr;
	}

	void ImguiPropertyFiltersWidget::update(ImguiDebugData& debugData)
	{
		if (!mIsInited)
		{
			init(debugData);
			mIsInited = true;
		}

		std::optional<std::vector<std::unique_ptr<AbstractPropertyFilter>>::const_iterator> filterToRemove;
		for (auto& filter : mAppliedFilters)
		{
			filter->updateImguiWidget();
			ImGui::SameLine();
			if (ImGui::Button(FormatString("x##%s", filter->getName()).c_str()))
			{
				filterToRemove = std::find(mAppliedFilters.begin(), mAppliedFilters.end(), filter);
			}
		}
		if (filterToRemove.has_value())
		{
			mAppliedFilters.erase(*filterToRemove);
		}

		if (ImGui::InputTextWithHint("##FilterSearch", "Search Query", mFilterQueryBuffer, IM_ARRAYSIZE(mFilterQueryBuffer)))
		{
			{
				Entity::EntityId id = 0;
				std::string_view strId(mFilterQueryBuffer, IM_ARRAYSIZE(mFilterQueryBuffer));
				std::stringstream ss;
				if (strId[0] == '0' && strId[1] == 'x') {
					ss << std::hex;
				}
				ss << strId;
				ss >> id;
				Entity entity(id);

				std::unordered_map<CellPos, WorldCell>& allCells = debugData.worldHolder.getWorld().getSpatialData().getAllCells();
				WorldCell* cell = findEntityCell(allCells, entity);
				if (cell != nullptr)
				{
					mExplicitlySetEntity = std::make_tuple(cell, entity);
					return;
				}
				else
				{
					mExplicitlySetEntity = std::nullopt;
				}
			}

			std::string strId(mFilterQueryBuffer, std::strlen(mFilterQueryBuffer));
			// tolower
			std::transform(strId.begin(), strId.end(), strId.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });

			if (strId.size() >= MinimalSearchLen)
			{
				mLastMatchedProperties = mPropertyDescriptors.findAllMatches(strId);
			}
			else
			{
				mLastMatchedProperties.clear();
				mSuggestedFiltersFactories.clear();
			}
		}

		bool isFiltersListShown = false;
		for (const auto& property : mLastMatchedProperties)
		{
			const std::string& propertyName = property->getName();
			ImGui::TextUnformatted(propertyName.c_str());
			ImGui::SameLine();
			if (ImGui::Button(FormatString("Add Filter##%s", propertyName).c_str()))
			{
				mSuggestedFiltersFactories.clear();
				for (const auto& factory : property->getFilterFactories())
				{
					mSuggestedFiltersFactories.push_back(factory);
				}
				isFiltersListShown = true;
				break;
			}
		}
		if (isFiltersListShown)
		{
			mLastMatchedProperties.clear();
			mFilterQueryBuffer[0] = '\0';
		}

		bool isFilterChosen = false;
		for (const auto& suggestedFilter : mSuggestedFiltersFactories)
		{
			std::string filterName = suggestedFilter->getName();
			ImGui::TextUnformatted(filterName.c_str());
			ImGui::SameLine();
			if (ImGui::Button(FormatString("Add##%s", filterName).c_str()))
			{
				isFilterChosen = true;
				mAppliedFilters.push_back(suggestedFilter->createFilter());
				break;
			}
		}
		if (isFilterChosen)
		{
			mSuggestedFiltersFactories.clear();
		}
	}

	std::vector<StringId> ImguiPropertyFiltersWidget::getFilteredComponentTypes() const
	{
		std::vector<StringId> filteredComponents;
		filteredComponents.reserve(mAppliedFilters.size());
		// construct vector of unique elements
		for (const auto& appliedFilter : mAppliedFilters)
		{
			StringId typeId = appliedFilter->getComponentType();
			auto lowerIt = std::lower_bound(filteredComponents.begin(), filteredComponents.end(), typeId);
			if (lowerIt == filteredComponents.end() || *lowerIt != typeId)
			{
				filteredComponents.insert(lowerIt, typeId);
			}
		}
		return filteredComponents;
	}

	static void getSpatialEntitiesHavingComponents(std::unordered_map<CellPos, WorldCell>& cells, const std::vector<StringId>& componentTypes, TupleVector<WorldCell*, Entity>& inOutEntities)
	{
		std::vector<Entity> entities;

		for (auto& [pos, cell] : cells)
		{
			EntityManager& singleThreadedManager = cell.getEntityManager();
			entities.clear();
			singleThreadedManager.getEntitiesHavingComponents(componentTypes, entities);
			std::transform(
				entities.begin(),
				entities.end(),
				std::back_inserter(inOutEntities),
				[&cell](Entity entity)
				{
					return std::make_tuple(&cell, entity);
				}
			);
		}
	}

	void ImguiPropertyFiltersWidget::getFilteredEntities(ImguiDebugData& debugData, TupleVector<WorldCell*, Entity>& inOutEntities)
	{
		inOutEntities.clear();

		std::vector<StringId> filteredComponentTypes = getFilteredComponentTypes();

		if (!filteredComponentTypes.empty())
		{
			std::unordered_map<CellPos, WorldCell>& allCells = debugData.worldHolder.getWorld().getSpatialData().getAllCells();
			getSpatialEntitiesHavingComponents(allCells, filteredComponentTypes, inOutEntities);

			for (const auto& filter : mAppliedFilters)
			{
				filter->filterEntities(inOutEntities);
			}
		}
		else if (mExplicitlySetEntity.has_value())
		{
			inOutEntities.push_back(*mExplicitlySetEntity);
		}
	}
} // namespace ImguiPropertyFiltration

#pragma once

#include <raccoon-ecs/entity.h>
#include <map>

#include "GameLogic/Imgui/ComponentInspector/AbstractComponentImguiWidget.h"
#include "GameLogic/Imgui/ComponentInspector/PropertyFilters/ImguiPropertyFiltersWidget.h"

class WorldHolder;
class WorldCell;
struct ImguiDebugData;

class ImguiComponentInspectorWindow
{
public:
	ImguiComponentInspectorWindow();

	void update(const ImguiDebugData& debugData);

	bool isVisible = false;

private:
	void applyFilters(const ImguiDebugData& debugData);

	void showEntityId() const;
	void showFilteredEntities();
	void showComponentsInspector();

private:
	char mEntityFilterBuffer[128] = "";
	std::optional<std::tuple<WorldCell*, Entity>> mSelectedEntity;
	TupleVector<WorldCell*, Entity> mFilteredEntities;
	std::map<StringId, std::unique_ptr<AbstractComponentImguiWidget>> mComponentInspectWidgets;
	ImguiPropertyFiltration::ImguiPropertyFiltersWidget mPropertyFiltersWidget;
};

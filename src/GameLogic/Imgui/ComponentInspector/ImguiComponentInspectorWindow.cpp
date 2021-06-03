#include "Base/precomp.h"

#include "GameLogic/Imgui/ComponentInspector/ImguiComponentInspectorWindow.h"

#include <algorithm>
#include <cstring>
#include <ranges>
#include <sstream>
#include <string_view>

#include "imgui/imgui.h"

#include <raccoon-ecs/component_factory.h>

#include "GameData/GameData.h"
#include "GameData/World.h"

#include "GameLogic/Imgui/ImguiDebugData.h"
#include "GameLogic/SharedManagers/WorldHolder.h"

#include "GameLogic/Imgui/ComponentInspector/ComponentWidgetsRegistration.h"

ImguiComponentInspectorWindow::ImguiComponentInspectorWindow()
{
	ComponentWidgetsRegistration::RegisterInspectWidgets(mComponentInspectWidgets);
}

static void HelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void ImguiComponentInspectorWindow::applyFilters(ImguiDebugData& debugData, const RaccoonEcs::InnerDataAccessor& dataAccessor)
{
	 mPropertyFiltersWidget.getFilteredEntities(debugData, dataAccessor, mFilteredEntities);
}

void ImguiComponentInspectorWindow::showEntityId()
{
	if (mSelectedEntity.has_value())
	{
		ImGui::Text("id:%llu", static_cast<unsigned long long>(std::get<1>(*mSelectedEntity).getId()));
		ImGui::SameLine();
		if (ImGui::Button("Copy"))
		{
			ImGui::LogToClipboard();
			ImGui::LogText("id:%llu", static_cast<unsigned long long>(std::get<1>(*mSelectedEntity).getId()));
			ImGui::LogFinish();
		}
	}
}

void ImguiComponentInspectorWindow::showFilteredEntities()
{
	if (!mFilteredEntities.empty())
	{
		if (ImGui::TreeNode("Filtered entities"))
		{
			ImGui::BeginGroup();
			auto scrollBoxSize = ImVec2(200.0f, std::min(180.0f, static_cast<float>(mFilteredEntities.size()) * 17.0f + ImGui::GetStyle().FramePadding.y*4));
			if (ImGui::BeginChild("FilteredEntities", scrollBoxSize, true))
			{
				for (auto & filteredEntity : mFilteredEntities)
				{
					const Entity& entity = std::get<1>(filteredEntity);
					char buf[32];
					sprintf(buf, "id:%llu", static_cast<unsigned long long>(entity.getId()));
					if (ImGui::Selectable(buf, mSelectedEntity.has_value() && std::get<1>(*mSelectedEntity) == entity))
					{
						mSelectedEntity = filteredEntity;
						std::strcpy(mEntityFilterBuffer, buf);
					}
				}
			}
			ImGui::EndChild();
			ImGui::EndGroup();
			ImGui::TreePop();
		}
	}
}

void ImguiComponentInspectorWindow::showComponentsInspector(const RaccoonEcs::InnerDataAccessor& dataAccessor)
{
	bool hasFoundAnything = false;
	if (mSelectedEntity.has_value())
	{
		auto [cell, entity] = *mSelectedEntity;

		std::vector<TypedComponent> components;
		dataAccessor.getSingleThreadedEntityManager(cell->getEntityManager()).getAllEntityComponents(entity, components);
		std::ranges::sort(components, [](const auto& a, const auto& b)
		{
			return a.typeId < b.typeId;
		});

		for (TypedComponent componentData : components)
		{
			std::string name = FormatString("%s##ComponentInspection", ID_TO_STR(componentData.typeId).c_str());
			if (ImGui::TreeNode(name.c_str()))
			{
				auto it = mComponentInspectWidgets.find(componentData.typeId);
				if (it != mComponentInspectWidgets.end())
				{
					it->second->update(componentData.component);
				}

				ImGui::TreePop();
				ImGui::Separator();
			}
			hasFoundAnything = true;
		}
	}

	if (!hasFoundAnything && mSelectedEntity.has_value())
	{
		ImGui::Text("No inspectable entity with such ID found");
		ImGui::SameLine(); HelpMarker("An entity without any components also can't be inspectable");
	}
}

void ImguiComponentInspectorWindow::update(ImguiDebugData& debugData, const RaccoonEcs::InnerDataAccessor& dataAccessor)
{
	if (isVisible)
	{
		ImGui::Begin("Component Inspector", &isVisible);

		mPropertyFiltersWidget.update(debugData, dataAccessor);

		applyFilters(debugData, dataAccessor);

		ImGui::Text("Entities matching the filter: %lu", mFilteredEntities.size());

		showFilteredEntities();

		showEntityId();

		showComponentsInspector(dataAccessor);

		ImGui::End();
	}
}

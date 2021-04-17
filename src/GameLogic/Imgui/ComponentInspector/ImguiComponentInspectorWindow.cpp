#include "Base/precomp.h"

#include "GameLogic/Imgui/ComponentInspector/ImguiComponentInspectorWindow.h"

#include <algorithm>
#include <cstring>
#include <ranges>
#include <sstream>
#include <string_view>

#include "imgui/imgui.h"

#include "ECS/ComponentFactory.h"

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

void ImguiComponentInspectorWindow::applyFilters(ImguiDebugData& debugData)
{
	 mPropertyFiltersWidget.getFilteredEntities(debugData, mFilteredEntities);
}

void ImguiComponentInspectorWindow::showEntityId()
{
	if (mSelectedEntity.has_value())
	{
		ImGui::Text("0x%x", std::get<1>(*mSelectedEntity).getID());
		ImGui::SameLine();
		if (ImGui::Button("Copy"))
		{
			ImGui::LogToClipboard();
			ImGui::LogText("0x%x", std::get<1>(*mSelectedEntity).getID());
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
					sprintf(buf, "0x%x", entity.getID());
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

void ImguiComponentInspectorWindow::showComponentsInspector()
{
	bool hasFoundAnything = false;
	if (mSelectedEntity.has_value())
	{
		auto [cell, entity] = *mSelectedEntity;

		std::vector<BaseComponent*> components;
		cell->getEntityManager().getAllEntityComponents(entity, components);
		std::ranges::sort(components, [](auto a, auto b)
		{
			return a->getComponentTypeName() < b->getComponentTypeName();
		});
		for (BaseComponent* component : components)
		{
			std::string name = FormatString("%s##ComponentInspection", ID_TO_STR(component->getComponentTypeName()).c_str());
			if (ImGui::TreeNode(name.c_str()))
			{
				auto it = mComponentInspectWidgets.find(component->getComponentTypeName());
				if (it != mComponentInspectWidgets.end())
				{
					it->second->update(component);
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

void ImguiComponentInspectorWindow::update(ImguiDebugData& debugData)
{
	if (isVisible)
	{
		ImGui::Begin("Component Inspector", &isVisible);

		mPropertyFiltersWidget.update(debugData);

		applyFilters(debugData);

		ImGui::Text("Entities matching the filter: %lu", mFilteredEntities.size());

		showFilteredEntities();

		showEntityId();

		showComponentsInspector();

		ImGui::End();
	}
}

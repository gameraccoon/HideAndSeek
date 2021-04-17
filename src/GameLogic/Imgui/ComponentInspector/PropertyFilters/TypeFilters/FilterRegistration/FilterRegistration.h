#pragma once

#include "GameData/Core/Vector2D.h"

#include "GameLogic/Imgui/ComponentInspector/PropertyFilters/AbstractPropertyFilter.h"

namespace ImguiPropertyFiltration
{
	template<typename T>
	void RegisterFilterFactories(std::vector<std::shared_ptr<AbstractPropertyFilterFactory>>& inOutFilterFactories, const std::shared_ptr<AbstractPropertyDescriptor>& descriptor);

	template<>
	void RegisterFilterFactories<float>(std::vector<std::shared_ptr<AbstractPropertyFilterFactory>>& inOutFilterFactories, const std::shared_ptr<AbstractPropertyDescriptor>& descriptor);

	template<>
	void RegisterFilterFactories<Vector2D>(std::vector<std::shared_ptr<AbstractPropertyFilterFactory>>& inOutFilterFactories, const std::shared_ptr<AbstractPropertyDescriptor>& descriptor);

	template<typename T>
	void RegisterFilterFactories(std::vector<std::shared_ptr<AbstractPropertyFilterFactory>>& /*inOutFilterFactories*/, const std::shared_ptr<AbstractPropertyDescriptor>& /*descriptor*/)
	{
		// fill nothing for unknown types
	}
}

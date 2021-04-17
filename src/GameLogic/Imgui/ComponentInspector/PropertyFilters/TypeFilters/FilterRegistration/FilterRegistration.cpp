#include "Base/precomp.h"

#include "GameLogic/Imgui/ComponentInspector/PropertyFilters/TypeFilters/FilterRegistration/FilterRegistration.h"

#include "GameLogic/Imgui/ComponentInspector/PropertyFilters/TypeFilters/FloatPropertyFilters.h"
#include "GameLogic/Imgui/ComponentInspector/PropertyFilters/TypeFilters/Vector2DPropertyFIlters.h"

namespace ImguiPropertyFiltration
{
	template<>
	void RegisterFilterFactories<float>(std::vector<std::shared_ptr<AbstractPropertyFilterFactory>>& inOutFilterFactories, const std::shared_ptr<AbstractPropertyDescriptor>& descriptor)
	{
		inOutFilterFactories.push_back(std::make_shared<PropertyFilterFactory<FloatGreaterPropertyFilter>>(descriptor));
		inOutFilterFactories.push_back(std::make_shared<PropertyFilterFactory<FloatLessPropertyFilter>>(descriptor));
	}

	template<>
	void RegisterFilterFactories<Vector2D>(std::vector<std::shared_ptr<AbstractPropertyFilterFactory>>& inOutFilterFactories, const std::shared_ptr<AbstractPropertyDescriptor>& descriptor)
	{
		inOutFilterFactories.push_back(std::make_shared<PropertyFilterFactory<Vector2DInRectPropertyFilter>>(descriptor));
	}
}

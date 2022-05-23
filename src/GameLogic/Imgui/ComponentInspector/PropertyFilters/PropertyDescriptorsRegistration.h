#pragma once

#include <memory>
#include <string>
#include <vector>

namespace ImguiPropertyFiltration
{
	class AbstractPropertyDescriptor;

	namespace PropertyDescriptorsRegistration
	{
		using DescriptionsRawData = std::vector<std::pair<std::vector<std::string>, std::shared_ptr<AbstractPropertyDescriptor>>>;

		DescriptionsRawData GetDescriptions();
	}
}

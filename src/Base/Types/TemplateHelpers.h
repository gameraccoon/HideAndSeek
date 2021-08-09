#pragma once

#include <algorithm>
#include <ranges>
#include <type_traits>
#include <variant>

namespace TemplateHelpers
{
	// copy or move depending on the container type category
	template<typename SourceContainerType, typename IteratorType>
	void CopyOrMoveContainer(const typename std::remove_reference<SourceContainerType>::type& sourceContainer, IteratorType&& outputIterator)
	{
		std::ranges::copy(sourceContainer, std::forward<IteratorType>(outputIterator));
	}

	// copy or move depending on the container type category
	template<typename SourceContainerType, typename IteratorType>
	void CopyOrMoveContainer(typename std::remove_reference<SourceContainerType>::type&& sourceContainer, IteratorType&& outputIterator)
	{
		std::ranges::move(sourceContainer, std::forward<IteratorType>(outputIterator));
	}

	template<typename T, typename Container, typename... Args>
	T& EmplaceVariant(Container& container, Args&&... args)
	{
		return std::get<T>(container.template emplace_back(std::in_place_type<T>, std::forward<Args>(args)...));
	}
}

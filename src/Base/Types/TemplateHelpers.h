#pragma once

#include <algorithm>
#include <ranges>
#include <type_traits>

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
}

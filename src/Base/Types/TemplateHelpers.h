#pragma once

#include <type_traits>
#include <algorithm>

namespace TemplateHelpers
{
	// copy or move depending on the container type category
	template<typename SourceContainerType, typename IteratorType>
	void CopyOrMoveContainer(const typename std::remove_reference<SourceContainerType>::type& sourceContainer, IteratorType&& outputIterator)
	{
		std::copy(sourceContainer.begin(), sourceContainer.end(), std::forward<IteratorType>(outputIterator));
	}

	// copy or move depending on the container type category
	template<typename SourceContainerType, typename IteratorType>
	void CopyOrMoveContainer(typename std::remove_reference<SourceContainerType>::type&& sourceContainer, IteratorType&& outputIterator)
	{
		std::move(sourceContainer.begin(), sourceContainer.end(), std::forward<IteratorType>(outputIterator));
	}
}

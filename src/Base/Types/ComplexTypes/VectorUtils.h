#pragma once

#include <limits>
#include <vector>

#include "Base/Types/TemplateHelpers.h"

namespace VectorUtils
{
	template <typename T>
	void RemoveIndexes(std::vector<T>& inOutVector, const std::vector<size_t>& sortedIndexesToRemove)
	{
		if (sortedIndexesToRemove.empty())
		{
			return;
		}

		const size_t vectorSize = inOutVector.size();
		const size_t indexesCount = sortedIndexesToRemove.size();
		size_t skippedIndexes = 0;
		for (size_t i = sortedIndexesToRemove[0] + 1, j = 1; i < vectorSize; ++i)
		{
			if (j < indexesCount && i == sortedIndexesToRemove[j])
			{
				++j;
				while (j < indexesCount && sortedIndexesToRemove[j] == i)
				{
					++j;
					++skippedIndexes;
				}
			}
			else
			{
				inOutVector[i - j + skippedIndexes] = std::move(inOutVector[i]);
			}
		}

		inOutVector.erase(inOutVector.begin() + (vectorSize - indexesCount + skippedIndexes), inOutVector.end());
	}

	template <typename T>
	void RemoveIndexesFast(std::vector<T>& inOutVector, const std::vector<size_t>& sortedIndexesToRemove)
	{
		const size_t vectorSize = inOutVector.size();
		const size_t indexesCount = sortedIndexesToRemove.size();
		size_t previousIndex = std::numeric_limits<size_t>::max();
		size_t skippedIndexes = 0;
		for (size_t i = 0; i < indexesCount; ++i)
		{
			size_t index = sortedIndexesToRemove[i];
			if (index == previousIndex)
			{
				++skippedIndexes;
				continue;
			}

			inOutVector[index] = std::move(inOutVector[vectorSize - i + skippedIndexes - 1]);
			previousIndex = index;
		}

		inOutVector.erase(inOutVector.begin() + (vectorSize - indexesCount + skippedIndexes), inOutVector.end());
	}

	template<typename First, typename... Others>
	auto JoinVectors(First&& firstVector, Others&&... otherVectors)
	{
		using value_type = typename std::decay<decltype(*firstVector.begin())>::type;
		std::vector<value_type> result;
		size_t size = firstVector.size() + (otherVectors.size() + ...);
		result.reserve(size);
		TemplateHelpers::CopyOrMoveContainer<First>(std::forward<First>(firstVector), std::back_inserter(result));
		(TemplateHelpers::CopyOrMoveContainer<Others>(std::forward<Others>(otherVectors), std::back_inserter(result)), ...);
		return result;
	}

	template<typename VectorBase, typename VectorAdded>
	void AppendToVector(VectorBase& base, VectorAdded&& added)
	{
		if (base.empty())
		{
			base = std::forward<VectorAdded>(added);
		}
		else
		{
			if constexpr (std::is_rvalue_reference<VectorAdded&&>::value)
			{
				std::move(
					added.begin(),
					added.end(),
					std::back_inserter(base)
				);
			}
			else
			{
				std::copy(
					added.begin(),
					added.end(),
					std::back_inserter(base)
				);
			}
		}
	}
}

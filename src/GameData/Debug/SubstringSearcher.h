#pragma once

#include <string>
#include <string_view>
#include <vector>

template<typename T>
class SubstringSearcher
{
public:
	void construct(std::vector<std::pair<std::vector<std::string>, T>>&& data)
	{
		mRelations = std::move(data);
	}

	template<typename Vec>
	void findAndFillAllMatches(Vec& outResult, std::string_view searchQuery) const noexcept
	{
		std::vector<size_t> valueIndexes;
		// the most straightforward solution for now
		for (const auto& [keys, value] : mRelations)
		{
			for (const std::string& key : keys)
			{
				if (key.find(searchQuery) != std::string::npos)
				{
					outResult.push_back(value);
					break;
				}
			}
		}
	}

	std::vector<T> findAllMatches(std::string_view searchQuery) const noexcept
	{
		std::vector<T> result;
		findAndFillAllMatches(result, searchQuery);
		return result;
	}

	std::vector<std::reference_wrapper<const T>> findAllMatchesRef(std::string_view searchQuery) const noexcept
	{
		std::vector<std::reference_wrapper<const T>> result;
		findAndFillAllMatches(result, searchQuery);
		return result;
	}

private:
	std::vector<std::pair<std::vector<std::string>, T>> mRelations;
};

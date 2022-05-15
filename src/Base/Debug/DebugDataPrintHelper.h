#pragma once

#include <unordered_map>
#include <vector>
#include <ostream>

namespace DebugDataPrintHelper
{
	// Print csv table with buckets and elements count for each bucket
	template<typename MapType>
	void PrintUnorderedMapBucketUtilization(const MapType& map, std::ostream& outStream)
	{
		for (size_t i = 0; i < map.bucket_count()-1; ++i)
		{
			outStream << i << ",";
		}
		outStream << map.bucket_count()-1 << "\n";

		for (size_t i = 0; i < map.bucket_count()-1; ++i)
		{
			outStream << map.bucket_size(i) << ",";
		}
		outStream << map.bucket_size(map.bucket_count()-1) << "\n";
	}

	// called after PrintUnorderedMapBucketUtilization to print values that lay in each bucket
	// printFn - function that returns a printable representation of key or value (or both) from std::pair<key, value>
	template<typename MapType, typename PrintFn>
	void PrintConflictingElementsForBucketStatistics(const MapType& map, PrintFn printFn, std::ostream& outStream)
	{
		std::unordered_map<size_t, std::vector<typename MapType::value_type>> bucketValueMapping;
		size_t maxSize = 0;
		for (auto& pair : map)
		{
			size_t bucket = map.bucket(pair.first);
			bucketValueMapping[bucket].push_back(pair);
			maxSize = std::max(maxSize, bucketValueMapping[bucket].size());
		}

		for (size_t row = 0; row < maxSize; ++row)
		{
			for (size_t i = 0; i < map.bucket_count()-1; ++i)
			{
				auto it = bucketValueMapping.find(i);
				if (it != bucketValueMapping.end() && row < it->second.size())
				{
					outStream << printFn(it->second[row]);
				}
				outStream << ",";
			}
			auto it = bucketValueMapping.find(map.bucket_count()-1);
			if (it != bucketValueMapping.end() && row < it->second.size())
			{
				outStream << printFn(it->second[row]);
			}
			outStream << "\n";
		}
	}
}

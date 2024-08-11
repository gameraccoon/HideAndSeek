#pragma once

#include <compare>
#include <functional>
#include <type_traits>

class ResourceHandle
{
public:
	using IndexType = int;

public:
	ResourceHandle() = default;
	explicit ResourceHandle(const IndexType index)
		: resourceIndex(index) {}

	std::strong_ordering operator<=>(const ResourceHandle& other) const = default;

	[[nodiscard]] bool isValid() const { return resourceIndex != InvalidResourceIndex; }

	static constexpr IndexType InvalidResourceIndex = -1;
	IndexType resourceIndex = InvalidResourceIndex;
};

template<>
struct std::hash<ResourceHandle>
{
	size_t operator()(const ResourceHandle resourceHandle) const noexcept
	{
		return hash<ResourceHandle::IndexType>()(resourceHandle.resourceIndex);
	}
};

static_assert(std::is_trivially_copyable<ResourceHandle>(), "ResourceHandle should be trivially copyable");

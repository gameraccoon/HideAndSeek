#pragma once

#include <type_traits>

class [[nodiscard]] ResourceHandle
{
public:
	using IndexType = int;

public:
	ResourceHandle() = default;
	explicit ResourceHandle(IndexType index) : resourceIndex(index) {}

	[[nodiscard]] bool isValid() const { return resourceIndex != InvalidResourceIndex; }

	static constexpr IndexType InvalidResourceIndex = -1;
	IndexType resourceIndex = InvalidResourceIndex;
};

static_assert(std::is_trivially_copyable<ResourceHandle>(), "ResourceHandle should be trivially copyable");

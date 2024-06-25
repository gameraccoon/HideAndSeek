#pragma once

#include <utility>

template<typename CallableType>
class ScopeFinalizer
{
public:
	explicit ScopeFinalizer(CallableType&& finalizeFn)
		: mFinalizeFn(std::move(finalizeFn))
	{}

	ScopeFinalizer(ScopeFinalizer&) = delete;
	ScopeFinalizer& operator=(ScopeFinalizer&) = delete;
	ScopeFinalizer(ScopeFinalizer&&) = delete;
	ScopeFinalizer& operator=(ScopeFinalizer&&) = delete;

	~ScopeFinalizer()
	{
		mFinalizeFn();
	}

private:
	CallableType mFinalizeFn;
};

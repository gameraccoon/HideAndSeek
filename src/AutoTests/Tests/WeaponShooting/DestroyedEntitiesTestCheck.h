#pragma once

#include "AutoTests/TestChecklist.h"

class DestroyedEntitiesTestCheck : public TestCheck
{
public:
	explicit DestroyedEntitiesTestCheck(int expectedDestroyedEntities)
		: mExpectedDestroyedEntities(expectedDestroyedEntities)
	{}

	[[nodiscard]] bool isPassed() const override { return mDestroyedEntities == mExpectedDestroyedEntities; };
	[[nodiscard]] std::string describe() const override
	{
		return FormatString("Destroyed entities: %d, expected %d.", mDestroyedEntities, mExpectedDestroyedEntities);
	}

	void addDestroyedEntities(int count) { mDestroyedEntities += count; }

private:
	int mDestroyedEntities = 0;
	int mExpectedDestroyedEntities;
};


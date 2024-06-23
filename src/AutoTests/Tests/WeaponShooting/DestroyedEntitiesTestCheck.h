#pragma once

#include "Base/Types/String/StringHelpers.h"

#include "AutoTests/BasicTestChecks.h"

class DestroyedEntitiesTestCheck final : public TestCheck
{
public:
	explicit DestroyedEntitiesTestCheck(const int expectedDestroyedEntities)
		: mExpectedDestroyedEntities(expectedDestroyedEntities)
	{}

	[[nodiscard]] bool hasBeenValidated() const override { return true; }
	[[nodiscard]] bool hasPassed() const override { return mDestroyedEntities == mExpectedDestroyedEntities; };
	[[nodiscard]] std::string getErrorMessage() const override
	{
		return FormatString("Destroyed entities: %d, expected %d.", mDestroyedEntities, mExpectedDestroyedEntities);
	}

	void addDestroyedEntities(const int count) { mDestroyedEntities += count; }

private:
	int mDestroyedEntities = 0;
	int mExpectedDestroyedEntities;
};


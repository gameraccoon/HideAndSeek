#pragma once

#include <memory>
#include <string>
#include <vector>

// Check BasicTestChecks.h for the usage examples
class TestCheck
{
public:
	virtual ~TestCheck() = default;
	// true means we have a result of the check
	[[nodiscard]] virtual bool hasBeenValidated() const = 0;
	// returns true if the check has been validated and passed
	[[nodiscard]] virtual bool hasPassed() const = 0;
	// returns the message that will be shown if the tests failed or was not validated
	[[nodiscard]] virtual std::string getErrorMessage() const = 0;
};

class TestChecklist
{
public:
	template<typename Check, typename... Args>
	Check& addCheck(Args&&... args)
	{
		mChecks.emplace_back(std::make_unique<Check>(std::forward<Args>(args)...));
		return static_cast<Check&>(*mChecks.back());
	}

	// returns false if at least one check was not validated
	[[nodiscard]] bool areAllChecksValidated() const
	{
		for (const auto& check : mChecks)
		{
			if (!check->hasBeenValidated())
			{
				return false;
			}
		}
		return true;
	}

	// returns true if at least one check was validated and failed
	[[nodiscard]] bool hasAnyCheckFailed() const
	{
		for (const auto& check : mChecks)
		{
			if (check->hasBeenValidated() && !check->hasPassed())
			{
				return true;
			}
		}
		return false;
	}

	[[nodiscard]] const std::vector<std::unique_ptr<TestCheck>>& getChecks() const
	{
		return mChecks;
	}

private:
	std::vector<std::unique_ptr<TestCheck>> mChecks;
};

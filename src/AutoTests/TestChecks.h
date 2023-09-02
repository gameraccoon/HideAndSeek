#pragma once

#include <memory>
#include <string>
#include <vector>

class TestCheck
{
public:
	virtual ~TestCheck() = default;
	[[nodiscard]] virtual bool wasChecked() const = 0;
	[[nodiscard]] virtual bool hasPassed() const = 0;
	[[nodiscard]] virtual std::string getErrorMessage() const = 0;
};

class SimpleTestCheck final : public TestCheck
{
public:
	SimpleTestCheck(const SimpleTestCheck&) = delete;
	SimpleTestCheck(SimpleTestCheck&&) = delete;

	template<typename Str, typename = std::enable_if_t<std::is_convertible_v<Str, std::string>>>
	explicit SimpleTestCheck(Str&& errorMessage)
		: mErrorMessage(std::forward<Str>(errorMessage))
	{}

	[[nodiscard]] bool hasPassed() const final { return mIsPassed; }
	[[nodiscard]] std::string getErrorMessage() const final { return mErrorMessage; }

	void checkAsPassed()
	{
		mIsPassed = true;
		mWasChecked = true;
	}

	void checkAsFailed()
	{
		mIsPassed = false;
		mWasChecked = true;
	}

	[[nodiscard]] bool wasChecked() const override { return mWasChecked; }

private:
	bool mIsPassed = false;
	bool mWasChecked = false;
	std::string mErrorMessage;
};

struct TestChecklist
{
	std::vector<std::unique_ptr<TestCheck>> checks;

	template<typename Str, typename = std::enable_if_t<std::is_convertible_v<Str, std::string>>>
	SimpleTestCheck& addSimpleCheck(Str&& errorMessage)
	{
		return addCheck<SimpleTestCheck>(std::forward<Str>(errorMessage));
	}

	template<typename Check, typename... Args>
	Check& addCheck(Args&&... args)
	{
		checks.emplace_back(std::make_unique<Check>(std::forward<Args>(args)...));
		return static_cast<Check&>(*checks.back());
	}

	[[nodiscard]] bool areAllChecked() const
	{
		for (const auto& check : checks)
		{
			if (!check->wasChecked())
			{
				return false;
			}
		}
		return true;
	}
};

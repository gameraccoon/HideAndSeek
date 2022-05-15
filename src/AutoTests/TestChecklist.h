#pragma once

class TestCheck
{
public:
	virtual ~TestCheck() = default;
	[[nodiscard]] virtual bool isPassed() const = 0;
	[[nodiscard]] virtual std::string describe() const = 0;
};

struct TestChecklist
{
	std::map<std::string, std::unique_ptr<TestCheck>> checks;
};

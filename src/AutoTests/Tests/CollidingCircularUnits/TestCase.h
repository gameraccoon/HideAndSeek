#pragma once

#include "Utils/Application/ArgumentsParser.h"

#include "AutoTests/BaseTestCase.h"

class CollidingCircularUnitsTestCase final : public BaseTestCase
{
public:
	using BaseTestCase::BaseTestCase;

protected:
	void initTestCase(const ArgumentsParser& arguments) override;
};

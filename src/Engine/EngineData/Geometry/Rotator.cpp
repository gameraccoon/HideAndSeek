#include "EngineCommon/precomp.h"

#include "EngineData/Geometry/Rotator.h"

#include <nlohmann/json.hpp>

#include "EngineCommon/Math/Float.h"

Rotator::Rotator(const float angle) noexcept
	: mValue(angle)
{
	normalize();
}

bool Rotator::isNearlyEqualTo(const Rotator other) const noexcept
{
	const float difference = this->getValue() - other.getValue();
	return Math::AreEqualWithEpsilon(difference, 0) || Math::AreEqualWithEpsilon(difference, TWO_PI);
}

bool Rotator::operator==(const Rotator other) const noexcept
{
	return getValue() == other.getValue();
}

bool Rotator::operator!=(const Rotator other) const noexcept
{
	return !(*this == other);
}

Rotator Rotator::operator+(const Rotator other) const noexcept
{
	return Rotator(getValue() + other.getValue());
}

Rotator Rotator::operator+=(const Rotator other) noexcept
{
	mValue += other.getValue();
	normalize();

	return *this;
}

Rotator Rotator::operator-(const Rotator right) const noexcept
{
	return Rotator(getValue() - right.getValue());
}

Rotator Rotator::operator-=(const Rotator right) noexcept
{
	mValue -= right.getValue();
	normalize();

	return *this;
}

float Rotator::NormalizeRawAngle(float rawAngle)
{
	while (rawAngle <= -PI)
	{
		rawAngle += 2 * PI;
	}
	while (rawAngle > PI)
	{
		rawAngle -= 2 * PI;
	}
	return rawAngle;
}

void Rotator::normalize() noexcept
{
	mValue = NormalizeRawAngle(mValue);
}

void to_json(nlohmann::json& outJson, const Rotator& rotator)
{
	outJson = nlohmann::json{{"v", rotator.mValue}};
}

void from_json(const nlohmann::json& json, Rotator& outRotator)
{
	json.at("v").get_to(outRotator.mValue);
}

static_assert(std::is_trivial<Rotator>(), "Rotator should be trivial type");
static_assert(std::is_standard_layout<Rotator>(), "Rotator should have standard layout");
static_assert(std::is_trivially_copyable<Rotator>(), "Rotator should be trivially copyable");

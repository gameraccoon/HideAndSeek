#include "Base/precomp.h"

#include "GameData/Geometry/Rotator.h"

#include <nlohmann/json.hpp>

#include "Base/Math/Float.h"

bool Rotator::isNearlyEqualTo(Rotator other) const noexcept
{
	float difference = this->getValue() - other.getValue();
	return Math::AreEqualWithEpsilon(difference, 0) || Math::AreEqualWithEpsilon(difference, TWO_PI);
}

bool Rotator::operator==(Rotator other) const noexcept
{
	return getValue() == other.getValue();
}

bool Rotator::operator!=(Rotator other) const noexcept
{
	return !(*this == other);
}

Rotator Rotator::operator+(Rotator other) const noexcept
{
	return Rotator(getValue() + other.getValue());
}

Rotator Rotator::operator+=(Rotator other) noexcept
{
	mValue += other.getValue();
	normalize();

	return *this;
}

Rotator Rotator::operator-(Rotator right) const noexcept
{
	return Rotator(getValue() - right.getValue());
}

Rotator Rotator::operator-=(Rotator right) noexcept
{
	mValue -= right.getValue();
	normalize();

	return *this;
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

#include "Base/precomp.h"

#include "GameData/Core/Border.h"


Border::Border(Vector2D a, Vector2D b) noexcept
	: mPointA(a)
	, mPointB(b)
	, normal((b - a).normal())
{
}

void Border::setA(Vector2D a) noexcept
{
	mPointA = a;
	calculateNormal();
}

void Border::setB(Vector2D b) noexcept
{
	mPointB = b;
	calculateNormal();
}

void Border::calculateNormal() noexcept
{
	normal = (mPointB - mPointA).normal();
}

static_assert(std::is_trivially_copyable<Border>(), "Border should be trivially copyable");

bool SimpleBorder::operator==(const SimpleBorder& other) const noexcept
{
	return (a == other.a && b == other.b);
}

bool SimpleBorder::operator!=(const SimpleBorder& other) const noexcept
{
	return !(*this == other);
}

void to_json(nlohmann::json& outJson, const SimpleBorder& border)
{
	outJson = nlohmann::json{
		{"a", border.a},
		{"b", border.b}
	};
}

void from_json(const nlohmann::json& json, SimpleBorder& border)
{
	json.at("a").get_to(border.a);
	json.at("b").get_to(border.b);
}

static_assert(std::is_pod<SimpleBorder>(), "SimpleBorder should be a POD type");
static_assert(std::is_trivially_constructible<SimpleBorder>(), "SimpleBorder should be trivially constructible");
static_assert(std::is_trivially_copyable<SimpleBorder>(), "SimpleBorder should be trivially copyable");

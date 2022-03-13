#pragma once

#include "nlohmann/json_fwd.hpp"

#include "GameData/Geometry/Vector2D.h"

class Border
{
public:
	Border(Vector2D a, Vector2D b) noexcept;

	/** Get normal-vector for this border */
	[[nodiscard]] Vector2D getNormal() const { return mNormal; }
	/** Get first point of this segment */
	[[nodiscard]] Vector2D getA() const { return mPointA; }
	/** Get second point of this segment */
	[[nodiscard]] Vector2D getB() const { return mPointB; }
	/** Change first point of this segment (and recalculate normal) */
	void setA(Vector2D a) noexcept;
	/** Change second point of this segment (and recalculate normal) */
	void setB(Vector2D b) noexcept;

private:
	void calculateNormal() noexcept;

private:
	Vector2D mPointA;
	Vector2D mPointB;
	/** Normal of this border */
	Vector2D mNormal;
};

struct SimpleBorder
{
	SimpleBorder() = default;
	SimpleBorder(Vector2D a, Vector2D b) : a(a), b(b) {}

	[[nodiscard]] bool operator==(const SimpleBorder& other) const noexcept;
	[[nodiscard]] bool operator!=(const SimpleBorder& other) const noexcept;

	friend void to_json(nlohmann::json& outJson, const SimpleBorder& border);
	friend void from_json(const nlohmann::json& json, SimpleBorder& border);

	Vector2D a;
	Vector2D b;
};

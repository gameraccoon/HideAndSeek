#pragma once

#include <cmath>
#include <cstring>

#include <nlohmann/json_fwd.hpp>

#include "Rotator.h"


struct Vector2D
{
	float x;
	float y;

	// leaves inner data uninitialized
	Vector2D() = default;
	// can be created from initializer list
	constexpr Vector2D(float x, float y) : x(x), y(y) {}
	explicit Vector2D(Rotator rotator) noexcept;

	/** Get vector length in units */
	[[nodiscard]] float size() const noexcept;

	/** Get quarter of vector length (faster than Size()) */
	[[nodiscard]] float qSize() const noexcept;

	[[nodiscard]] bool isZeroLength() const noexcept;

	/** Normalize vector */
	[[nodiscard]] Vector2D unit() const noexcept;
	/** Mirror horizontally */
	[[nodiscard]] Vector2D mirrorH() const noexcept;
	/** Mirror vertically */
	[[nodiscard]] Vector2D mirrorV() const noexcept;
	/** Get normal-vector */
	[[nodiscard]] Vector2D normal() const noexcept;

	/** Project this vector to line that parallel with the vector "base" */
	[[nodiscard]] Vector2D project(Vector2D base) const noexcept;

	/** Check that current point is inside an AABB rect with the given top-left and bottom-right points */
	[[nodiscard]] bool isInsideRect(Vector2D lt, Vector2D rb) const noexcept;

	/** Get angle between vector and OX axis */
	[[nodiscard]] Rotator rotation() const noexcept;

	/** Get a copy of this vector rotated about origin */
	[[nodiscard]] Vector2D getRotated(Rotator rotation) const;

	// check for exact equality
	[[nodiscard]] bool operator==(Vector2D other) const noexcept;
	[[nodiscard]] bool operator!=(Vector2D other) const noexcept;

	[[nodiscard]] bool isNearlyEqualTo(Vector2D other) const noexcept;
	[[nodiscard]] bool isNearlyEqualTo(Vector2D other, float error) const noexcept;

	[[nodiscard]] Vector2D operator-() const noexcept;

	[[nodiscard]] Vector2D operator+(Vector2D other) const noexcept;
	Vector2D operator+=(Vector2D other) noexcept;

	[[nodiscard]] Vector2D operator-(Vector2D right) const noexcept;
	Vector2D operator-=(Vector2D right) noexcept;

	[[nodiscard]] Vector2D operator*(float scalar) const noexcept;
	[[nodiscard]] friend Vector2D operator*(float scalar, Vector2D vector) noexcept;
	Vector2D operator*=(float scalar) noexcept;

	[[nodiscard]] Vector2D operator/(float scalar) const noexcept;
	Vector2D operator/=(float scalar) noexcept;

	[[nodiscard]] static float DotProduct(Vector2D left, Vector2D right) noexcept;
	[[nodiscard]] static Vector2D HadamardProduct(Vector2D left, Vector2D right) noexcept;

	[[nodiscard]] static Vector2D Lerp(Vector2D left, Vector2D right, float t);
	/** If the given point is not on the line betwen two point then the result is not specified */
	[[nodiscard]] static float InvLerp(Vector2D left, Vector2D right, Vector2D point);

	friend void to_json(nlohmann::json& outJson, const Vector2D& vector);
	friend void from_json(const nlohmann::json& json, Vector2D& outVector);
};

constexpr Vector2D LEFT_DIRECTION(-1.0f, 0.0f);
constexpr Vector2D RIGHT_DIRECTION(1.0f, 0.0f);
constexpr Vector2D UP_DIRECTION(0.0f, -1.0f);
constexpr Vector2D DOWN_DIRECTION(0.0f, 1.0f);
constexpr Vector2D ZERO_VECTOR(0.0f, 0.0f);

// if we want to use vector as key in unordered map then we can use this class to allow to be compared with some precision
template<int PrecisionBin = 10>
struct Vector2DKey
{
	static constexpr float Multiplier = static_cast<float>(1 << PrecisionBin);

	struct Key {
		long x;
		long y;
	};

	Key key;
	Vector2D value;

	constexpr explicit Vector2DKey(Vector2D value)
		: key(static_cast<long>(std::round(value.x * Multiplier)), static_cast<long>(std::round(value.y * Multiplier)))
		, value(value)
	{}

	[[nodiscard]] constexpr bool operator==(const Vector2DKey& other) const noexcept
	{
		return key.x == other.key.x && key.y == other.key.y;
	}

	[[nodiscard]] constexpr bool operator<(const Vector2DKey& other) const noexcept
	{
		return std::memcmp(&key, &other.key, sizeof(Key)) < 0;
	}

	Vector2D calcRoundedValue() const {
		return Vector2D{static_cast<float>(key.x) / Multiplier, static_cast<float>(key.y) / Multiplier};
	}
};

namespace std
{
	template <>
	struct hash<Vector2D>
	{
		std::size_t operator()(Vector2D k) const
		{
			return hash<float>()(k.x) ^ std::rotl(hash<float>()(k.y), 7);
		}
	};

	template <int Precision>
	struct hash<Vector2DKey<Precision>>
	{
		std::size_t operator()(const Vector2DKey<Precision>& k) const
		{
			return hash<long>()(k.key.x) ^ std::rotl(hash<long>()(k.key.y), 7);
		}
	};
}

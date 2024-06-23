#include "Base/precomp.h"

#include "GameData/Geometry/Vector2D.h"

#include <nlohmann/json.hpp>

static constexpr float VECTOR_ERROR = 0.0001f;

Vector2D::Vector2D(const Rotator rotator) noexcept
{
	const float angle = rotator.getValue();
	x = std::cos(angle);
	y = std::sin(angle);
}

float Vector2D::size() const noexcept
{
	return sqrt((x * x) + (y * y));
}

float Vector2D::qSize() const noexcept
{
	return (x * x) + (y * y);
}

bool Vector2D::isZeroLength() const noexcept
{
	return x == 0.0f && y == 0.0f;
}

Vector2D Vector2D::unit() const noexcept
{
	if (isZeroLength())
	{
		return ZERO_VECTOR;
	}

	return (*this) / size();
}

Rotator Vector2D::rotation() const noexcept
{
	return Rotator(atan2(y, x));
}

Vector2D Vector2D::getRotated(const Rotator rotation) const
{
	const float angle = rotation.getValue();
	const float sine = std::sin(angle);
	const float cosine = std::cos(angle);
	return Vector2D(cosine * x - sine * y, sine * x + cosine * y);
}

Vector2D Vector2D::mirrorH() const noexcept
{
	return Vector2D(-x, y);
}

Vector2D Vector2D::mirrorV() const noexcept
{
	return Vector2D(x, -y);
}

Vector2D Vector2D::normal() const noexcept
{
	const float sizeInverse = 1.0f / size();
	return Vector2D(y * sizeInverse, -x * sizeInverse);
}

Vector2D Vector2D::project(const Vector2D base) const noexcept
{
	const float qSize = base.qSize();
	Vector2D result(ZERO_VECTOR);

	if (qSize != 0.0f)
	{
		const float dProduct = DotProduct(base, (*this));
		result = base * dProduct / qSize;
	}

	return result;
}

bool Vector2D::isInsideRect(const Vector2D lt, const Vector2D rb) const noexcept
{
	return x >= lt.x && x <= rb.x && y >= lt.y && y <= rb.y;
}

Vector2D Vector2D::operator-() const noexcept
{
	return Vector2D(-x, -y);
}

bool Vector2D::isNearlyEqualTo(const Vector2D other) const noexcept
{
	return (std::fabs(x - other.x) + std::fabs(y - other.y)) <= VECTOR_ERROR;
}

bool Vector2D::isNearlyEqualTo(const Vector2D other, const float error) const noexcept
{
	return (std::fabs(x - other.x) + std::fabs(y - other.y)) <= error;
}

Vector2D Vector2D::operator+(const Vector2D other) const noexcept
{
	return Vector2D(x + other.x, y + other.y);
}

Vector2D Vector2D::operator+=(const Vector2D other) noexcept
{
	x += other.x;
	y += other.y;

	return *this;
}

Vector2D Vector2D::operator-(const Vector2D right) const noexcept
{
	return Vector2D(x - right.x, y - right.y);
}

Vector2D Vector2D::operator-=(const Vector2D right) noexcept
{
	x -= right.x;
	y -= right.y;

	return *this;
}

Vector2D Vector2D::operator*(const float scalar) const noexcept
{
	return Vector2D(x * scalar, y * scalar);
}

Vector2D Vector2D::operator*=(const float scalar) noexcept
{
	x *= scalar;
	y *= scalar;

	return *this;
}

Vector2D Vector2D::operator/(const float scalar) const noexcept
{
	return Vector2D(x / scalar, y / scalar);
}

Vector2D Vector2D::operator/=(const float scalar) noexcept
{
	x /= scalar;
	y /= scalar;

	return *this;
}

float Vector2D::DotProduct(const Vector2D left, const Vector2D right) noexcept
{
	return left.x * right.x + left.y * right.y;
}

Vector2D Vector2D::HadamardProduct(const Vector2D left, const Vector2D right) noexcept
{
	return Vector2D(left.x * right.x, left.y * right.y);
}

Vector2D Vector2D::Lerp(const Vector2D left, const Vector2D right, float t)
{
	return Vector2D(left.x + (right.x - left.x) * t, left.y + (right.y - left.y) * t);
}

float Vector2D::InvLerp(const Vector2D left, const Vector2D right, const Vector2D point)
{
	const float distanceX = right.x - left.x;
	const float distanceY = right.y - left.y;
	if (std::fabs(distanceX) > std::fabs(distanceY))
	{
		return (point.x - left.x) / distanceX;
	}
	else
	{
		return (point.y - left.y) / distanceY;
	}
}

Vector2D operator*(const float scalar, const Vector2D vector) noexcept
{
	return Vector2D(scalar * vector.x, scalar * vector.y);
}

void to_json(nlohmann::json& outJson, const Vector2D& vector)
{
	outJson = nlohmann::json{{"x", vector.x}, {"y", vector.y}};
}

void from_json(const nlohmann::json& json, Vector2D& outVector)
{
	json.at("x").get_to(outVector.x);
	json.at("y").get_to(outVector.y);
}

std::size_t std::hash<Vector2D>::operator()(const Vector2D k) const noexcept
{
	return hash<float>()(k.x) ^ std::rotl(hash<float>()(k.y), 7);
}

static_assert(sizeof(Vector2D) == sizeof(int)*2, "Vector2D is too big");
static_assert(std::is_trivial<Vector2D>(), "Vector2D should be trivial type");
static_assert(std::is_standard_layout<Vector2D>(), "Vector2D should have standard layout");
static_assert(std::is_trivially_copyable<Vector2D>(), "Vector2D should be trivially copyable");

static_assert(std::is_trivially_copyable<Vector2DKey<>>(), "Vector2DKey<> should be trivially copyable");
static_assert(std::is_standard_layout<Vector2DKey<>>(), "Vector2DKey<> should have standard layout");
static_assert(std::is_trivially_copyable<Vector2DKey<>>(), "Vector2DKey<> should be trivially copyable");

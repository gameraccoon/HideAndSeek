#include "Base/precomp.h"

#include "GameData/Core/Vector2D.h"

#include <cmath>

#include <nlohmann/json.hpp>

static const float VECTOR_ERROR = 0.0001f;

const Vector2D LEFT_DIRECTION(-1.0f, 0.0f);
const Vector2D RIGHT_DIRECTION(1.0f, 0.0f);
const Vector2D UP_DIRECTION(0.0f, -1.0f);
const Vector2D DOWN_DIRECTION(0.0f, 1.0f);
const Vector2D ZERO_VECTOR(0.0f, 0.0f);


Vector2D::Vector2D(Rotator rotator) noexcept
{
	float angle = rotator.getValue();
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
	return Vector2D(rotation() - Rotator(HalfPI));
}

Vector2D Vector2D::project(Vector2D base) const noexcept
{
	float qSize = base.qSize();
	Vector2D result(ZERO_VECTOR);

	if (qSize != 0.0f)
	{
		float dProduct = DotProduct(base, (*this));
		result = base * dProduct / qSize;
	}

	return result;
}

bool Vector2D::isInsideRect(Vector2D lt, Vector2D rb) const noexcept
{
	return x >= lt.x && x <= rb.x && y >= lt.y && y <= rb.y;
}

Vector2D Vector2D::operator-() const noexcept
{
	return Vector2D(-x, -y);
}

bool Vector2D::operator==(Vector2D other) const noexcept
{
	return x == other.x && y == other.y;
}

bool Vector2D::operator!=(Vector2D other) const noexcept
{
	return !(*this == other);
}

bool Vector2D::isNearlyEqualTo(Vector2D other) const noexcept
{
	return (std::fabs(x - other.x) + std::fabs(y - other.y)) <= VECTOR_ERROR;
}

Vector2D Vector2D::operator+(Vector2D other) const noexcept
{
	return Vector2D(x + other.x, y + other.y);
}

Vector2D Vector2D::operator+=(Vector2D other) noexcept
{
	x += other.x;
	y += other.y;

	return *this;
}

Vector2D Vector2D::operator-(Vector2D right) const noexcept
{
	return Vector2D(x - right.x, y - right.y);
}

Vector2D Vector2D::operator-=(Vector2D right) noexcept
{
	x -= right.x;
	y -= right.y;

	return *this;
}

Vector2D Vector2D::operator*(float scalar) const noexcept
{
	return Vector2D(x * scalar, y * scalar);
}

Vector2D operator*(float scalar, Vector2D vector) noexcept
{
	return Vector2D(scalar * vector.x, scalar * vector.y);
}

Vector2D Vector2D::operator*=(float scalar) noexcept
{
	x *= scalar;
	y *= scalar;

	return *this;
}

Vector2D Vector2D::operator/(float scalar) const noexcept
{
	return Vector2D(x / scalar, y / scalar);
}

Vector2D Vector2D::operator/=(float scalar) noexcept
{
	x /= scalar;
	y /= scalar;

	return *this;
}

float Vector2D::DotProduct(const Vector2D& left, const Vector2D& right) noexcept
{
	return left.x * right.x + left.y * right.y;
}

Vector2D Vector2D::HadamardProduct(const Vector2D& left, const Vector2D& right) noexcept
{
	return Vector2D(left.x * right.x, left.y * right.y);
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

static_assert(sizeof(Vector2D) == sizeof(int)*2, "Vector2D is too big");
static_assert(std::is_pod<Vector2D>(), "Vector2D should be pod type");
static_assert(std::is_trivially_copyable<Vector2D>(), "Vector2D should be trivially copyable");

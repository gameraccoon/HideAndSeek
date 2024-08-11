#include "EngineCommon/precomp.h"

#include <algorithm>

#include <gtest/gtest.h>

#include "EngineCommon/Types/TemplateAliases.h"

#include "GameData/Components/LightBlockingGeometryComponent.generated.h"
#include "GameData/Geometry/Hull.h"

#include "GameUtils/Geometry/VisibilityPolygon.h"

static bool AreVisibilityPolygonsEqual(const std::vector<Vector2D>& a, const std::vector<Vector2D>& b)
{
	constexpr float allowedError = 0.001f;

	if (a.empty() && b.empty())
	{
		return true;
	}

	if (b.empty() || a.empty())
	{
		return false;
	}

	auto aCopy = a;

	const auto it = std::ranges::find_if(aCopy, [start = b.front()](const Vector2D val) { return val.isNearlyEqualTo(start, allowedError); });
	if (it == aCopy.end())
	{
		return false;
	}

	std::rotate(aCopy.begin(), it, aCopy.end());
	return aCopy.size() == b.size() && std::equal(aCopy.begin(), aCopy.end(), b.begin(), [](const Vector2D a, const Vector2D b) { return a.isNearlyEqualTo(b, allowedError); });
}

static void FillTestLightBlockingGeometry(LightBlockingGeometryComponent& component, const Hull& geometry, const Vector2D& location)
{
	std::vector<SimpleBorder>& borders = component.getBordersRef();
	for (const Border& border : geometry.borders)
	{
		borders.emplace_back(border.getA() + location, border.getB() + location);
	}
}

TEST(VisibilityPolygon, TestPlygonsEqualityTest)
{
	EXPECT_TRUE(AreVisibilityPolygonsEqual({ { 30.0f, 30.0f }, { -50.0f, 50.0f }, { -10.0f, -10.0f } }, { { 30.0f, 30.0f }, { -50.0f, 50.0f }, { -10.0f, -10.0f } }));
	EXPECT_TRUE(AreVisibilityPolygonsEqual({ { 30.0f, 30.0f }, { -50.0f, 50.0f }, { -10.0f, -10.0f } }, { { -50.0f, 50.0f }, { -10.0f, -10.0f }, { 30.0f, 30.0f } }));
	EXPECT_TRUE(AreVisibilityPolygonsEqual({ { 30.0f, 30.0f }, { -50.0f, 50.0f }, { -10.0f, -10.0f } }, { { -10.0f, -10.0f }, { 30.0f, 30.0f }, { -50.0f, 50.0f } }));
	EXPECT_FALSE(AreVisibilityPolygonsEqual({ { 30.0f, 30.0f }, { -50.0f, 50.0f }, { -10.0f, -10.0f } }, { { -10.0f, -10.0f }, { -50.0f, 50.0f }, { 30.0f, 30.0f } }));
	EXPECT_FALSE(AreVisibilityPolygonsEqual({ { 30.0f, 30.0f }, { -50.0f, 50.0f }, { -10.0f, -10.0f } }, { { 30.0f, 30.0f }, { -50.0f, 50.0f } }));
	EXPECT_FALSE(AreVisibilityPolygonsEqual({ { 30.0f, 30.0f }, { -50.0f, 50.0f } }, { { 30.0f, 30.0f }, { -50.0f, 50.0f }, { -10.0f, -10.0f } }));
}

TEST(VisibilityPolygon, Empty)
{
	VisibilityPolygonCalculator calc;
	const std::vector<const LightBlockingGeometryComponent*> components;
	const Vector2D pos(-10.0f, 20.0f);

	std::vector<Vector2D> result;
	calc.calculateVisibilityPolygon(result, components, pos, Vector2D(600.0f, 600.0f));
	EXPECT_TRUE(AreVisibilityPolygonsEqual(result, { { -300.0f, -300.0f }, { 300.0f, -300.0f }, { 300.0f, 300.0f }, { -300.0f, 300.0f } }));
}

TEST(VisibilityPolygon, OneBorder)
{
	VisibilityPolygonCalculator calc;
	constexpr Vector2D pos(-10.0f, -40.0f);

	Hull geometry;
	geometry.type = HullType::Angular;
	geometry.points = { { -60.0f, -50.0f }, { 60.0f, -50.0f }, { 60.0f, 50.0f }, { -60.0f, 50.0f } };
	geometry.generateBorders();
	constexpr Vector2D location(-10.0f, -210.0f);
	std::vector<const LightBlockingGeometryComponent*> components;
	LightBlockingGeometryComponent component;
	FillTestLightBlockingGeometry(component, geometry, location);
	components.push_back(&component);

	std::vector<Vector2D> result;
	calc.calculateVisibilityPolygon(result, components, pos, Vector2D(600.0f, 600.0f));
	EXPECT_TRUE(AreVisibilityPolygonsEqual(result, { { -300, -300 }, { -150, -300 }, { -60, -120 }, { 60, -120 }, { 150, -300 }, { 300, -300 }, { 300, 300 }, { -300, 300 } }));
}

TEST(VisibilityPolygon, OneBorderEqualAngles)
{
	VisibilityPolygonCalculator calc;
	constexpr Vector2D pos(308.0f, -33.0f);

	Hull geometry;
	geometry.type = HullType::Angular;
	geometry.points = { { 60.0f, -60.0f }, { 60.0f, 60.0f }, { -60.0f, 60.0f }, { -60.0f, -60.0f } };
	geometry.generateBorders();
	constexpr Vector2D location(260.0f, 39.0f);
	std::vector<const LightBlockingGeometryComponent*> components;
	LightBlockingGeometryComponent component;
	FillTestLightBlockingGeometry(component, geometry, location);
	components.push_back(&component);

	std::vector<Vector2D> result;
	calc.calculateVisibilityPolygon(result, components, pos, Vector2D(600.0f, 600.0f));
	EXPECT_TRUE(AreVisibilityPolygonsEqual(result, { { -300.0f, -300.0f }, { 300.0f, -300.0f }, { 300.0f, 300.0f }, { 12.0f, 12.0f }, { -108.0f, 12.0f }, { -300.0f, 33.3333f } }));
}

TEST(VisibilityPolygon, ApertureDirection1)
{
	VisibilityPolygonCalculator calc;
	constexpr Vector2D pos(0.0f, 0.0f);

	std::vector<const LightBlockingGeometryComponent*> components;
	LightBlockingGeometryComponent component;
	{
		Hull geometry;
		geometry.type = HullType::Angular;
		geometry.points = { { -110.0f, -10.0f }, { 110.0f, -10.0f }, { 110.0f, 10.0f }, { -110.0f, 10.0f } };
		constexpr Vector2D location(-90.0f, -190.0f);
		// create 8 blocks that makes a figure like aperture
		for (int i = 0; i < 8; ++i)
		{
			const Rotator rotator(0.25f * static_cast<float>(i) * PI);
			auto rotatedGeometry = geometry;
			Vector2D rotatedLocation = location.getRotated(rotator);
			// rotate around zero point
			for (auto& point : rotatedGeometry.points)
			{
				point = point.getRotated(rotator);
			}
			rotatedGeometry.generateBorders();
			FillTestLightBlockingGeometry(component, rotatedGeometry, rotatedLocation);
		}
	}
	components.push_back(&component);

	std::vector<Vector2D> result;
	calc.calculateVisibilityPolygon(result, components, pos, Vector2D(600.0f, 600.0f));
	// a saw-like pattern
	EXPECT_TRUE(AreVisibilityPolygonsEqual(
		result,
		{
			{ -180.0f, -20.0f },
			{ -229.1026f, -25.4558f },
			{ -113.137f, -141.42137f },
			{ -144.0f, -180.0f },
			{ 20.0f, -180.0f },
			{ 25.4558f, -229.1026f },
			{ 141.42137f, -113.137f },
			{ 180.0f, -144.0f },
			{ 180.0f, 20.0f },
			{ 229.1026f, 25.4558f },
			{ 113.137f, 141.42137f },
			{ 144.0f, 180.0f },
			{ -20.0f, 180.0f },
			{ -25.4558f, 229.1026f },
			{ -141.42137f, 113.137f },
			{ -180.0f, 144.0f },
		}
	));
}

TEST(VisibilityPolygon, ApertureDirection2)
{
	VisibilityPolygonCalculator calc;
	constexpr Vector2D pos(0.0f, 0.0f);

	std::vector<const LightBlockingGeometryComponent*> components;
	LightBlockingGeometryComponent component;
	{
		Hull geometry;
		geometry.type = HullType::Angular;
		geometry.points = { { -110.0f, -10.0f }, { 110.0f, -10.0f }, { 110.0f, 10.0f }, { -110.0f, 10.0f } };
		constexpr Vector2D location(90.0f, -190.0f);
		// create 8 blocks that makes a figure like aperture
		for (int i = 0; i < 8; ++i)
		{
			const Rotator rotator(-0.25f * static_cast<float>(i) * PI);
			auto rotatedGeometry = geometry;
			Vector2D rotatedLocation = location.getRotated(rotator);
			// rotate around zero point
			for (auto& point : rotatedGeometry.points)
			{
				point = point.getRotated(rotator);
			}
			rotatedGeometry.generateBorders();
			FillTestLightBlockingGeometry(component, rotatedGeometry, rotatedLocation);
		}
	}
	components.push_back(&component);

	std::vector<Vector2D> result;
	calc.calculateVisibilityPolygon(result, components, pos, Vector2D(600.0f, 600.0f));
	// a saw-like pattern
	EXPECT_TRUE(AreVisibilityPolygonsEqual(
		result,
		{
			{ 180.0f, 144.0f },
			{ 141.42137f, 113.137f },
			{ 25.4558f, 229.1026f },
			{ 20.0f, 180.0f },
			{ -144.0f, 180.0f },
			{ -113.137f, 141.42137f },
			{ -229.1026f, 25.4558f },
			{ -180.0f, 20.0f },
			{ -180.0f, -144.0f },
			{ -141.42137f, -113.137f },
			{ -25.4558f, -229.1026f },
			{ -20.0f, -180.0f },
			{ 144.0f, -180.0f },
			{ 113.137f, -141.42137f },
			{ 229.1026f, -25.4558f },
			{ 180.0f, -20.0f },
		}
	));
}

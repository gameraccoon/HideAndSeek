#include "Base/precomp.h"

#include "Utils/Geometry/Collide.h"

#include <stdlib.h>
#include <cmath>
#include <limits>

#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"

#include "Base/Math/Float.h"

namespace Collide
{
	static constexpr float EPS = 1E-4f;

	enum class ResistDir
	{
		Normal,
		PointA,
		PointB
	};

	bool IsCollideGeometry(const Hull& hull1, const Hull& hull2, const Vector2D& center1, const Vector2D& center2, Vector2D& outResist)
	{
		// circle vs circle
		if (hull1.type == HullType::Circular && hull2.type == HullType::Circular)
		{
			float dist = (center2 - center1).qSize() - (hull1.getQRadius() + hull2.getQRadius() + 2.0f * hull1.getRadius() * hull2.getRadius());
			if (Math::IsLessWithEpsilon(dist, 0.0f))
			{
				outResist = (center2 - center1) - (center2 - center1).unit() * (hull1.getRadius() + hull2.getRadius());
				return true;
			}
			return false;
		}
		// circle vs rect
		else if (hull1.type == HullType::Circular || hull2.type == HullType::Circular)
		{
			const Hull *cHull, *rHull;
			const Vector2D *cCenter, *rCenter;
			if (hull1.type == HullType::Circular)
			{
				cHull = &hull1;
				cCenter = &center1;
				rHull = &hull2;
				rCenter = &center2;
			}
			else
			{
				cHull = &hull2;
				cCenter = &center2;
				rHull = &hull1;
				rCenter = &center1;
			}

			const Border *nearestBorder = nullptr;
			float nearestBorderQDistance = std::numeric_limits<float>::max();
			ResistDir nearestBorderResistDir = ResistDir::Normal;
			for (auto& border : rHull->borders)
			{
				Vector2D borderA = *rCenter + border.getA();

				float qDistance = Vector2D::DotProduct(border.getNormal(), borderA - *cCenter);
				qDistance*=qDistance;

				if (qDistance < nearestBorderQDistance)
				{
					ResistDir resistDir = ResistDir::Normal;

					Vector2D borderB = *rCenter + border.getB();
					float distA = (*cCenter - borderA).qSize();
					float distB = (*cCenter - borderB).qSize();

					// find nearest point
					if (distA < distB)
					{
						// check if we outside the section
						if (Vector2D::DotProduct(*cCenter - borderA, border.getB() - border.getA()) < 0)
						{
							qDistance = distA;
							resistDir = ResistDir::PointA;
						}
					}
					else
					{
						if (Vector2D::DotProduct(*cCenter - borderB, border.getA() - border.getB()) < 0)
						{
							qDistance = distB;
							resistDir = ResistDir::PointB;
						}
					}

					if (qDistance < nearestBorderQDistance)
					{
						nearestBorder = &border;
						nearestBorderQDistance = qDistance;
						nearestBorderResistDir = resistDir;
					}
				}
			}

			// if not intersecting the border
			if (nearestBorderQDistance >= cHull->getQRadius())
			{
				// if we outside the figure
				if (nearestBorder == nullptr
					|| nearestBorderResistDir != ResistDir::Normal
					|| SignedArea(*cCenter,
						*rCenter + nearestBorder->getA(),
						*rCenter + nearestBorder->getB()) <= 0.0f)
				{
					return false;
				}
			}

			if (nearestBorder == nullptr)
			{
				return false;
			}

			// we're touching the border or fully inside the figure
			Vector2D resist = ZERO_VECTOR;
			switch (nearestBorderResistDir)
			{
			case ResistDir::Normal:
				resist = nearestBorder->getNormal() * (sqrt(nearestBorderQDistance) - cHull->getRadius());
				break;
			case ResistDir::PointA:
			{
				Vector2D diffA = (*cCenter - *rCenter - nearestBorder->getA());
				resist = diffA - diffA.unit() * cHull->getRadius();
			}
				break;
			case ResistDir::PointB:
			{
				Vector2D diffB = (*cCenter - *rCenter - nearestBorder->getB());
				resist = diffB - diffB.unit() * cHull->getRadius();
				break;
			}
			default:
				break;
			}

			if (cHull == &hull1)
			{
				outResist=-resist;
			}
			else
			{
				outResist=resist;
			}
			return true;
		}
		else
		{
			// ToDo :some checks here
		}
		return false;
	}

	bool DoCollide(const CollisionComponent* collisionA, const Vector2D& locationA,
		const CollisionComponent* collisionB, const Vector2D& locationB, Vector2D& outResist)
	{
		// get AABB of the actors
		const BoundingBox box = collisionA->getBoundingBox() + locationA;
		const BoundingBox ourBox = collisionB->getBoundingBox() + locationB;
		// if the actor's AABB intersects with the Man's AABB (in new Man location)
		if (AreAABBsIntersect(box, ourBox))
		{
			return IsCollideGeometry(collisionA->getGeometry(), collisionB->getGeometry(),
				locationA, locationB, outResist);
		}
		return false;
	}

	void UpdateBoundingBox(CollisionComponent* collision)
	{
		const Hull& geometry = collision->getGeometry();

		if (geometry.type == HullType::Circular)
		{
			float radius = geometry.getRadius();
			collision->setBoundingBox(BoundingBox(Vector2D(-radius, -radius), Vector2D(radius, radius)));
		}
		else
		{
			float minX = 1000000;
			float maxX = -1000000;
			float minY = 1000000;
			float maxY = -1000000;

			for (auto point : geometry.points)
			{
				if (point.x < minX)
				{
					minX = point.x;
				}

				if (point.x > maxX)
				{
					maxX = point.x;
				}

				if (point.y < minY)
				{
					minY = point.y;
				}

				if (point.y > maxY)
				{
					maxY = point.y;
				}
			}

			collision->setBoundingBox(BoundingBox(Vector2D(minX, minY), Vector2D(maxX, maxY)));
		}
	}

	static int GetCohenCode(const BoundingBox& box, const Vector2D& dot)
	{
		constexpr int LEFT_BIT = 0;
		constexpr int RIGHT_BIT = 1;
		constexpr int TOP_BIT = 2;
		constexpr int BOTTOM_BIT = 3;

		return (
			((dot.x < box.minX) << LEFT_BIT)
			|
			((dot.x > box.maxX) << RIGHT_BIT)
			|
			((dot.y < box.minY) << TOP_BIT)
			|
			((dot.y > box.maxY) << BOTTOM_BIT)
		);
	}

	bool AreAABBsIntersect(const BoundingBox& boxA, const BoundingBox& boxB)
	{
		return (boxA.minX < boxB.maxX && boxA.maxX > boxB.minX)
			&& (boxA.minY < boxB.maxY && boxA.maxY > boxB.minY);
	}

	bool AreLinesIntersect(const Vector2D& A1, const Vector2D& A2, const Vector2D& B1, const Vector2D& B2)
	{
		return (
			// check that points B1 and B2 on the different sides of A1 A2 line
			Collide::SignedArea(A1, A2, B1) * Collide::SignedArea(A1, A2, B2) <= 0.0f
			&&
			// check that points A1 and A2 on the different sides of B1 B2 line
			Collide::SignedArea(B1, B2, A1) * Collide::SignedArea(B1, B2, A2) <= 0.0f
		);
	}

	bool AreLinesParallel(const Vector2D& A1, const Vector2D& A2, const Vector2D& B1, const Vector2D& B2)
	{
		Vector2D diffA = A2 - A1;
		Vector2D diffB = B2 - B1;
		if (diffA.y == 0 || diffB.y == 0)
		{
			if (diffA.x == 0 || diffB.x == 0)
			{
				return false;
			}
			return Math::AreEqualWithEpsilon(diffA.y/diffA.x, diffB.y/diffB.x);
		}
		else
		{
			return Math::AreEqualWithEpsilon(diffA.x/diffA.y, diffB.x/diffB.y);
		}
	}

	bool IsLineIntersectAABB(const BoundingBox& box, const Vector2D& start, const Vector2D& finish)
	{
		// get Cohen's code for start point
		int codeA = GetCohenCode(box, start);
		// get Cohen's code for end point
		int codeB = GetCohenCode(box, finish);

		// if the points on the same side of BB
		if ((codeA & codeB) != 0)
		{
			return false;
		}

		// one point is in BB another is out BB
		if ((codeA == 0 || codeB == 0) && (codeA | codeB) != 0)
		{
			return true;
		}

		// points on opposite sides of BB // 0011 or 1100
		if ((codeA | codeB) == 3 || (codeA | codeB) == 12)
		{
			return true;
		}

		float l = box.minX;
		float t = box.minY;
		float r = box.maxX;
		float b = box.maxY;

		float x1 = start.x;
		float y1 = start.y;
		float x2 = finish.x;
		float y2 = finish.y;

		// ToDo: optimize it for axis-aligned borders
		if (AreLinesIntersect(Vector2D(l, t), Vector2D(l, b), Vector2D(x1, y1), Vector2D(x2, y2))
			||
			AreLinesIntersect(Vector2D(r, t), Vector2D(r, b), Vector2D(x1, y1), Vector2D(x2, y2))
			||
			AreLinesIntersect(Vector2D(l, t), Vector2D(r, t), Vector2D(x1, y1), Vector2D(x2, y2))
			||
			AreLinesIntersect(Vector2D(l, b), Vector2D(r, b), Vector2D(x1, y1), Vector2D(x2, y2)))
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	static float Det(const float a, const float b, const float c, const float d)
	{
		return a * d - b * c;
	}

	Vector2D GetPointIntersect2Lines(const Vector2D& A1, const Vector2D& A2, const Vector2D& B1, const Vector2D& B2)
	{
		float DA1 = A1.y - A2.y;
		float DB1 = A2.x - A1.x;
		float DC1 = -DA1 * A1.x - DB1 * A1.y;
		float DA2 = B1.y - B2.y;
		float DB2 = B2.x - B1.x;
		float DC2 = -DA2 * B1.x - DB2 * B1.y;

		float zn = Det(DA1, DB1, DA2, DB2);

		// if lines are not parallel
		if (zn < -EPS || zn > EPS)
		{
			float x = -Det(DC1, DB1, DC2, DB2) / zn;
			float y = -Det(DA1, DC1, DA2, DC2) / zn;

			return Vector2D(x, y);
		}

		// if lines not intersected
		return ZERO_VECTOR;
	}
}

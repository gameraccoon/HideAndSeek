#include "Base/precomp.h"

#include "Utils/Geometry/ShapeOperations.h"

#include <algorithm>
#include <cstring>
#include <numeric>
#include <ranges>
#include <unordered_map>
#include <unordered_set>

#include "polypartition.h"

#include "Base/Math/Float.h"

#include "Utils/Geometry/Collide.h"

namespace ShapeOperations
{
	enum class SidePosition : short
	{
		Outside,
		Inside,
		OutsideOverride,
		InsideOverride,
		Unknown
	};

	enum class PointPositionOnSide
	{
		Start,
		Middle,
		End,
		Unknown
	};

	struct CutDirection
	{
		CutDirection(SidePosition firstSidePosition, SidePosition secondSidePosition, PointPositionOnSide otherBorderPointPosition)
			: firstSidePosition(firstSidePosition)
			, secondSidePosition(secondSidePosition)
			, otherBorderPointPosition(otherBorderPointPosition)
		{}

		SidePosition firstSidePosition;
		SidePosition secondSidePosition;
		PointPositionOnSide otherBorderPointPosition;
	};

	struct BorderPoint
	{
		explicit BorderPoint(Vector2D pos)
			: pos(pos)
			, cutDirection(SidePosition::Unknown, SidePosition::Unknown, PointPositionOnSide::Unknown)
		{}

		explicit BorderPoint(Vector2D pos, CutDirection cutDirection)
			: pos(pos)
			, cutDirection(cutDirection)
		{}

		Vector2D pos;
		CutDirection cutDirection;
	};

	struct BorderIntersection
	{
		BorderIntersection(size_t intersectionPoint, CutDirection cutDirection)
			: intersectionPoint(intersectionPoint)
			, cutDirection(cutDirection)
		{}

		size_t intersectionPoint;
		CutDirection cutDirection;
	};

	size_t addIntersectionPoint(Vector2D intersectionPoint, std::unordered_set<Vector2D>& intersectionPointsSet, std::vector<Vector2D>& intersectionPoints)
	{
		auto insertionResult = intersectionPointsSet.insert(intersectionPoint);

		if (!insertionResult.second)
		{
			auto it = std::find(intersectionPoints.begin(), intersectionPoints.end(), intersectionPoint);
			if (it != intersectionPoints.end())
			{
				return std::distance(intersectionPoints.begin(), it);
			}
			else
			{
				ReportError("intersectionPointsSet and intersectionPoints has diverged");
				return 0;
			}
		}
		else
		{
			intersectionPoints.push_back(intersectionPoint);
			return intersectionPoints.size() - 1;
		}
	}

	static bool FindBordersRotationAroundPoint(SimpleBorder border1, SimpleBorder border2, Vector2D intersectionPoint)
	{
		bool isCheckInversed = false;
		Vector2D firstPoint;
		Vector2D secondPoint;

		if (border1.a.isNearlyEqualTo(intersectionPoint))
		{
			firstPoint = border1.b;
			isCheckInversed = !isCheckInversed;
		}
		else
		{
			firstPoint = border1.a;
		}

		if (border2.b.isNearlyEqualTo(intersectionPoint))
		{
			secondPoint = border2.a;
			isCheckInversed = !isCheckInversed;
		}
		else
		{
			secondPoint = border2.b;
		}

		float signedArea = Collide::SignedArea(firstPoint, secondPoint, intersectionPoint);

		return isCheckInversed ? (signedArea < 0) : (signedArea > 0);
	}

	static Vector2D ChangePointBasis(const std::array<float, 4>& matrix, Vector2D point)
	{
		return Vector2D(
			point.x * matrix[0] + point.y * matrix[1],
			point.x * matrix[2] + point.y * matrix[3]
		);
	}

	static SimpleBorder ChangeBorderBasis(const std::array<float, 4>& baisisTransformMatrix, const SimpleBorder& border)
	{
		return SimpleBorder(ChangePointBasis(baisisTransformMatrix, border.a), ChangePointBasis(baisisTransformMatrix, border.b));
	}

	static const std::array<float, 4> InverseMatrix(std::array<float, 4> matrix)
	{
		const float determinant = matrix[0]*matrix[3] - matrix[1]*matrix[2];
		Assert(determinant != 0.0f, "Determinant should never be equal to zero");
		const float inverseDeterminant = 1/determinant;

		return { matrix[3] * inverseDeterminant, -matrix[1] * inverseDeterminant, -matrix[2] * inverseDeterminant, matrix[0] * inverseDeterminant };
	}

	static bool IsInside(float value, float a, float b)
	{
		return std::min(a, b) < value && value <= std::max(a, b);
	}

	static PointPositionOnSide GetPointPositionOnSide(Vector2D start, Vector2D end, Vector2D point, bool shouldInverse)
	{
		if (point == start)
		{
			return shouldInverse ? PointPositionOnSide::End : PointPositionOnSide::Start;
		}
		else if (point == end)
		{
			return shouldInverse ? PointPositionOnSide::Start : PointPositionOnSide::End;
		}
		else
		{
			return PointPositionOnSide::Middle;
		}
	}

	static void AddOverlappingLinesIntersectionPoints(
			const SimpleBorder& borderA,
			const SimpleBorder& borderB,
			std::unordered_set<Vector2D>& intersectionPointsSet,
			std::vector<Vector2D>& intersectionPoints,
			std::vector<BorderIntersection>& borderAIntersections,
			std::vector<BorderIntersection>& borderBIntersections)
	{
		// project borderB into coordigante system based on a borderA
		Vector2D basisX = (borderA.b - borderA.a).unit();
		Vector2D basisY = -basisX.normal();
		std::array<float, 4> basisTransformMatrix = InverseMatrix({basisX.x, basisY.x, basisX.y, basisY.y});
		SimpleBorder transformedA = ChangeBorderBasis(basisTransformMatrix, borderA);
		SimpleBorder transformedB = ChangeBorderBasis(basisTransformMatrix, borderB);
		Assert(transformedA.a.x < transformedA.b.x, "The line segment should have the same direction with the 0X origin");
		// check borderB direction
		const bool bHasSameDirection = transformedB.a.x < transformedB.b.x;

		if (Math::AreEqualWithEpsilon(transformedA.b.x, std::min(transformedB.a.x, transformedB.b.x)))
		{
			// don't need to do anything here, skip
		}
		else if (Math::AreEqualWithEpsilon(transformedA.a.x, std::max(transformedB.a.x, transformedB.b.x)))
		{
			// don't need to do anything here, skip
		}
		else
		{
			if (IsInside(transformedB.a.x, transformedA.a.x, transformedA.b.x))
			{
				size_t pointIdx = addIntersectionPoint(borderB.a, intersectionPointsSet, intersectionPoints);

				borderAIntersections.emplace_back(
					pointIdx,
					CutDirection(
						bHasSameDirection ? SidePosition::Unknown : SidePosition::InsideOverride,
						bHasSameDirection ? SidePosition::OutsideOverride : SidePosition::Unknown,
						bHasSameDirection ? PointPositionOnSide::Start : PointPositionOnSide::End
					)
				);

				borderBIntersections.emplace_back(
					pointIdx,
					CutDirection(
						SidePosition::Unknown,
						SidePosition::InsideOverride,
						GetPointPositionOnSide(borderA.a, borderA.b, borderB.a, !bHasSameDirection)
					)
				);
			}

			if (IsInside(transformedB.b.x, transformedA.a.x, transformedA.b.x))
			{
				size_t pointIdx = addIntersectionPoint(borderB.b, intersectionPointsSet, intersectionPoints);

				borderAIntersections.emplace_back(
					pointIdx,
					CutDirection(
						bHasSameDirection ? SidePosition::OutsideOverride : SidePosition::Unknown,
						bHasSameDirection ? SidePosition::Unknown : SidePosition::InsideOverride,
						bHasSameDirection ? PointPositionOnSide::End : PointPositionOnSide::Start
					)
				);

				borderBIntersections.emplace_back(
					pointIdx,
					CutDirection(
						SidePosition::InsideOverride,
						SidePosition::Unknown,
						GetPointPositionOnSide(borderA.a, borderA.b, borderB.b, !bHasSameDirection)
					)
				);
			}

			if (IsInside(transformedA.a.x, transformedB.a.x, transformedB.b.x))
			{
				size_t pointIdx = addIntersectionPoint(borderA.a, intersectionPointsSet, intersectionPoints);

				borderAIntersections.emplace_back(
					pointIdx,
					CutDirection(
						SidePosition::Unknown,
						bHasSameDirection ? SidePosition::OutsideOverride : SidePosition::InsideOverride,
						GetPointPositionOnSide(borderB.a, borderB.b, borderA.a, !bHasSameDirection)
					)
				);

				borderBIntersections.emplace_back(
					pointIdx,
					CutDirection(
						bHasSameDirection ? SidePosition::Unknown : SidePosition::InsideOverride,
						bHasSameDirection ? SidePosition::InsideOverride : SidePosition::Unknown,
						bHasSameDirection ? PointPositionOnSide::Start : PointPositionOnSide::End
					)
				);
			}

			if (IsInside(transformedA.b.x, transformedB.a.x, transformedB.b.x))
			{
				size_t pointIdx = addIntersectionPoint(borderA.b, intersectionPointsSet, intersectionPoints);

				borderAIntersections.emplace_back(
					pointIdx,
					CutDirection(
						bHasSameDirection ? SidePosition::OutsideOverride : SidePosition::InsideOverride,
						SidePosition::Unknown,
						GetPointPositionOnSide(borderB.a, borderB.b, borderA.a, !bHasSameDirection)
					)
				);

				borderBIntersections.emplace_back(
					pointIdx,
					CutDirection(
						bHasSameDirection ? SidePosition::InsideOverride : SidePosition::Unknown,
						bHasSameDirection ? SidePosition::Unknown : SidePosition::InsideOverride,
						bHasSameDirection ? PointPositionOnSide::End : PointPositionOnSide::Start
					)
				);
			}
		}
	}

	template<typename Func>
	static SidePosition GetBetterSidePosition(SidePosition oldPosition, SidePosition newPosition, Func shouldResolveToNew)
	{
		if (newPosition == SidePosition::Unknown)
		{
			return oldPosition;
		}

		if (newPosition == SidePosition::Inside && oldPosition == SidePosition::Outside)
		{
			return shouldResolveToNew() ? newPosition : oldPosition;
		}

		if (newPosition == SidePosition::Outside && oldPosition == SidePosition::Inside)
		{
			return shouldResolveToNew() ? newPosition : oldPosition;
		}

		Assert(newPosition != SidePosition::InsideOverride || oldPosition != SidePosition::OutsideOverride, "Conflicting side positions, this should not happen");
		Assert(newPosition != SidePosition::OutsideOverride || oldPosition != SidePosition::InsideOverride, "Conflicting side positions, this should not happen");

		if (oldPosition == SidePosition::InsideOverride || oldPosition == SidePosition::OutsideOverride)
		{
			return oldPosition;
		}

		return newPosition;
	}

	static bool IsFirstBorderGoesFirst(const PointPositionOnSide firstPos, const PointPositionOnSide secondPos)
	{
		Assert(firstPos != PointPositionOnSide::Unknown, "If we got to this function with Unknown point position on side, something went wrong");
		Assert(secondPos != PointPositionOnSide::Unknown, "If we got to this function with Unknown point position on side, something went wrong");
		Assert(firstPos != PointPositionOnSide::Middle, "If we got to this function with point position in the middle, something is wrong with the shape");
		Assert(secondPos != PointPositionOnSide::Middle, "If we got to this function with point position in the middle, something is wrong with the shape");
		return (firstPos == PointPositionOnSide::End && secondPos == PointPositionOnSide::Start);
	}

	static CutDirection CalcBetterCutDirection(const CutDirection oldDirection, const CutDirection newDirection)
	{
		auto firstSidePredicate = [newPos = newDirection.otherBorderPointPosition, oldPos = oldDirection.otherBorderPointPosition]
		{
			return !IsFirstBorderGoesFirst(newPos, oldPos);
		};

		auto secondSidePredicate = [newPos = newDirection.otherBorderPointPosition, oldPos = oldDirection.otherBorderPointPosition]
		{
			return IsFirstBorderGoesFirst(newPos, oldPos);
		};

		return CutDirection(
			GetBetterSidePosition(
				oldDirection.firstSidePosition,
				newDirection.firstSidePosition,
				firstSidePredicate
			),
			GetBetterSidePosition(
				oldDirection.secondSidePosition,
				newDirection.secondSidePosition,
				secondSidePredicate
			),
			newDirection.otherBorderPointPosition
		);
	}

	static SidePosition GetSidePostionForNormalCut(bool isOutside)
	{
		return isOutside ? SidePosition::Outside : SidePosition::Inside;
	}

	static bool IsSideOutside(SidePosition sidePos)
	{
		return sidePos == SidePosition::Outside || sidePos == SidePosition::OutsideOverride;
	}

	std::vector<SimpleBorder> GetUnion(const std::vector<SimpleBorder>& shape1, const std::vector<SimpleBorder>& shape2)
	{
		const size_t shape1BordersCount = shape1.size();
		std::vector<Vector2D> intersectionPoints;
		std::unordered_set<Vector2D> intersectionPointsSet;
		std::unordered_map<size_t, std::vector<BorderIntersection>> borderIntersections;

		// find all intersections
		for (size_t i = 0; i < shape1.size(); ++i)
		{
			SimpleBorder border = shape1[i];
			// create items even for the borders without intersections
			std::vector<BorderIntersection>& intersections = borderIntersections[i];
			for (size_t j = 0; j < shape2.size(); ++j)
			{
				SimpleBorder otherBorder = shape2[j];
				if (!Collide::AreLinesIntersect(border.a, border.b, otherBorder.a, otherBorder.b))
				{
					continue;
				}

				if (Collide::AreLinesParallel(border.a, border.b, otherBorder.a, otherBorder.b))
				{
					AddOverlappingLinesIntersectionPoints(
						border,
						otherBorder,
						intersectionPointsSet,
						intersectionPoints,
						intersections,
						borderIntersections[shape1BordersCount + j]
					);
				}
				else
				{
					Vector2D intersectionPoint = Collide::GetPointIntersect2Lines(border.a, border.b, otherBorder.a, otherBorder.b);

					// we can calculate whether a border part inside the resulting figure or outside from calculating
					// on which point of another border lies it point, we can do that by determining whether any three points
					// are in clockwise or counterclockwise order
					bool isClockwiseRotation = FindBordersRotationAroundPoint(border, otherBorder, intersectionPoint);

					size_t pointIdx = addIntersectionPoint(intersectionPoint, intersectionPointsSet, intersectionPoints);
					intersections.emplace_back(
						pointIdx,
						CutDirection(
							GetSidePostionForNormalCut(isClockwiseRotation),
							GetSidePostionForNormalCut(!isClockwiseRotation),
							GetPointPositionOnSide(otherBorder.a, otherBorder.b, intersectionPoint, isClockwiseRotation)
						)
					);

					borderIntersections[shape1BordersCount + j].emplace_back(
						pointIdx,
						CutDirection(
							GetSidePostionForNormalCut(!isClockwiseRotation),
							GetSidePostionForNormalCut(isClockwiseRotation),
							GetPointPositionOnSide(border.a, border.b, intersectionPoint, isClockwiseRotation)
						)
					);
				}
			}
		}

		{
			// create empty items for the borders without intersections
			for (size_t j = 0; j < shape2.size(); ++j)
			{
				borderIntersections.try_emplace(shape1BordersCount + j);
			}
		}

		std::vector<SimpleBorder> fracturedBorders;
		std::unordered_map<Vector2D, std::vector<size_t>> borderConnections;

		// split borders into smaller parts
		for (const auto& [borderIndex, intersections] : borderIntersections)
		{
			const SimpleBorder& border = (borderIndex < shape1BordersCount) ? shape1[borderIndex] : shape2[borderIndex - shape1BordersCount];

			std::vector<BorderPoint> borderPoints({
				BorderPoint(border.a),
				BorderPoint(border.b)
			});
			std::vector<float> borderPointFractions({0.0f, 1.0f});
			for (auto [intersectionIdx, cutDirection] : intersections)
			{
				const Vector2D intersectionPoint = intersectionPoints[intersectionIdx];
				const float intersectionFraction = Vector2D::InvLerp(border.a, border.b, intersectionPoint);
				// ToDo: replace binary search with linear (it's likely to have a very small amount of intersections)
				const auto it = std::lower_bound(borderPointFractions.begin(), borderPointFractions.end(), intersectionFraction);
				const size_t newPosition = std::distance(borderPointFractions.begin(), it);

				if (newPosition >= 1 && Math::AreEqualWithEpsilon(borderPointFractions[newPosition - 1], intersectionFraction))
				{
					borderPoints[newPosition - 1].cutDirection = CalcBetterCutDirection(borderPoints[newPosition - 1].cutDirection, cutDirection);
				}
				else if (newPosition < borderPointFractions.size() && Math::AreEqualWithEpsilon(borderPointFractions[newPosition], intersectionFraction))
				{
					borderPoints[newPosition].cutDirection = CalcBetterCutDirection(borderPoints[newPosition].cutDirection, cutDirection);
				}
				else
				{
					borderPointFractions.insert(it, intersectionFraction);
					borderPoints.emplace(borderPoints.begin() + newPosition, intersectionPoint, cutDirection);
				}
			}

			AssertFatal(!borderPoints.empty(), "borderPoints should always contain at least two elements");
			for (size_t i = 1; i < borderPoints.size(); ++i)
			{
				fracturedBorders.emplace_back(borderPoints[i - 1].pos, borderPoints[i].pos);

				if (!IsSideOutside(borderPoints[i - 1].cutDirection.secondSidePosition)
					&& !IsSideOutside(borderPoints[i].cutDirection.firstSidePosition))
				{
					borderConnections[fracturedBorders.back().a].push_back(fracturedBorders.size() - 1);
				}
			}
		}

		// find all chains of borders that should be eliminated
		std::vector<size_t> bordersToRemove;
		for (Vector2D intersectionPoint : intersectionPoints)
		{
			const std::vector<size_t>& connections = borderConnections[intersectionPoint];
			for (size_t startBorderIdx : connections)
			{
				if (fracturedBorders[startBorderIdx].a == intersectionPoint)
				{
					size_t borderIdx = startBorderIdx;
					while(true) // exit in the middle
					{
						bordersToRemove.push_back(borderIdx);
						Vector2D nextPoint = fracturedBorders[borderIdx].b;
						const std::vector<size_t>& nextConnections = borderConnections[nextPoint];
						size_t nextConnectionsCount = nextConnections.size();

						if (nextConnectionsCount != 1)
						{
							break;
						}

						if (intersectionPointsSet.find(nextPoint) != intersectionPointsSet.end())
						{
							break;
						}

						borderIdx = nextConnections[0];
					}
				}
			}
		}

		std::ranges::sort(bordersToRemove, std::greater());
		bordersToRemove.erase(std::unique(bordersToRemove.begin(), bordersToRemove.end()), bordersToRemove.end());

		for (size_t index : bordersToRemove)
		{
			fracturedBorders.erase(fracturedBorders.begin() + index);
		}

		// remove any duplicated borders
		// ToDo: probably need to correct the logic above to normally elliminate such borders
		// instead of doing this NlogN overcomplicated and not always correct (-0) stuff to fight a super-rare case
		static_assert(sizeof(SimpleBorder) == sizeof(float)*4, "SimpleBorder should have size of 4 floats");
		std::ranges::sort(fracturedBorders, [](const SimpleBorder& left, const SimpleBorder& right) {
			return std::memcmp(&left, &right, sizeof(SimpleBorder)) < 0;
		});
		auto last = std::unique(fracturedBorders.begin(), fracturedBorders.end());
		fracturedBorders.erase(last, fracturedBorders.end());

		return fracturedBorders;
	}

	struct BorderInfo
	{
		BorderInfo(Vector2D secondBorderPoint, size_t borderIndex, bool isFirstPoint)
			: secondBorderPoint(secondBorderPoint)
			, borderIndex(borderIndex)
			, isFirstPoint(isFirstPoint)
		{}

		Vector2D secondBorderPoint;
		size_t borderIndex;
		bool isFirstPoint;
	};

	void OptimizeShape(std::vector<SimpleBorder>& inOutShape)
	{
		// remove empty borders
		std::erase_if(
			inOutShape,
			[](const SimpleBorder& border) { return border.a == border.b; }
		);

		// collect neighboring borders
		std::unordered_map<Vector2DKey<>, std::vector<BorderInfo>> points;
		for (size_t i = 0, iSize = inOutShape.size(); i < iSize; ++i)
		{
			const SimpleBorder& border = inOutShape[i];
			points[border.a].emplace_back(border.b, i, true);
			points[border.b].emplace_back(border.a, i, false);
		}

		// iterate over all neighboring border pairs and join ones that produce a straight line
		std::vector<size_t> bordersToRemove;
		while (!points.empty())
		{
			auto& [pos, borders] = *points.begin();

			const size_t bordersCount = borders.size();
			// iterate over pairs
			for (size_t i = 1; i < bordersCount; ++i)
			{
				for (size_t j = 0; j < i; ++j)
				{
					// skip borders that have opposite directions
					if (borders[i].isFirstPoint != borders[j].isFirstPoint)
					{
						// process borders that produce a straight line
						const float area = Collide::SignedArea(pos.value, borders[i].secondBorderPoint, borders[j].secondBorderPoint);
						if (Math::IsNearZero(area))
						{
							const size_t finalBorderIdx = borders[i].borderIndex;
							const size_t borderIdxToRemove = borders[j].borderIndex;
							const Vector2D notChangedBorderPoint = borders[i].secondBorderPoint;
							const Vector2D movedBorderPoint = borders[j].secondBorderPoint;

							{
								Vector2D secondFinalBorderPoint;
								// replace border i with the merged border in the final shape
								SimpleBorder& finalBorder = inOutShape[finalBorderIdx];
								if (finalBorder.a.isNearlyEqualTo(pos.value))
								{
									finalBorder.a = movedBorderPoint;
									secondFinalBorderPoint = finalBorder.b;
								}
								else
								{
									finalBorder.b = movedBorderPoint;
									secondFinalBorderPoint = finalBorder.a;
								}

								// update secondBorderPoint of the intersection from another side of the final border
								auto pointsIt = points.find(secondFinalBorderPoint);
								// ignore if the other border was already processed
								if (pointsIt != points.end())
								{
									for (auto& borderInfo : pointsIt->second)
									{
										if (borderInfo.borderIndex == finalBorderIdx)
										{
											borderInfo.secondBorderPoint = movedBorderPoint;
											break;
										}
									}
								}
							}

							// link the final border instead of the removed border
							{
								const SimpleBorder borderToRemove = inOutShape[borderIdxToRemove];
								const Vector2D anotherIntersectionPoint = (borderToRemove.a == pos) ? borderToRemove.b : borderToRemove.a;
								auto pointsIt = points.find(anotherIntersectionPoint);
								// ignore if the other border was already processed
								if (pointsIt != points.end())
								{
									for (auto& borderInfo : pointsIt->second)
									{
										if (borderInfo.borderIndex == borderIdxToRemove)
										{
											borderInfo.borderIndex = finalBorderIdx;
											borderInfo.secondBorderPoint = notChangedBorderPoint;
											break;
										}
									}
								}
							}
							// border j is unlinked so we won't refer to it anymore, but we'll delete it later
							bordersToRemove.push_back(borderIdxToRemove);

							// there can't be more than one valid straight line going through one point
							goto pair_loop_exit;
						}
					}
				}
			}
			pair_loop_exit:

			points.erase(points.begin());
		}

		// remove extra borders that have left
		std::ranges::sort(bordersToRemove, std::greater());
		for (size_t index : bordersToRemove)
		{
			inOutShape.erase(inOutShape.begin() + index);
		}
	}

	std::vector<SimpleBorder> ConvertBordersToSimpleBorders(const std::vector<Border>& borders, Vector2D location)
	{
		std::vector<SimpleBorder> result(borders.size());
		std::transform(
			borders.begin(),
			borders.end(),
			result.begin(),
			[location](const Border& border)
			{
				return SimpleBorder(border.getA() + location, border.getB() + location);
			}
		);
		return result;
	}

	bool AreShapesIntersect(const MergedGeometry& firstGeometry, const MergedGeometry& secondGeometry)
	{
		if (!Collide::AreAABBsIntersectInclusive(firstGeometry.aabb, secondGeometry.aabb))
		{
			return false;
		}

		for (SimpleBorder firstBorder : firstGeometry.borders)
		{
			for (SimpleBorder secondBorder : secondGeometry.borders)
			{
				if (Collide::AreLinesIntersect(firstBorder.a, firstBorder.b, secondBorder.a, secondBorder.b))
				{
					return true;
				}
			}
		}

		return false;
	}

	void MergeGeometry(std::vector<MergedGeometry>& inOutGeometry)
	{
		if (inOutGeometry.empty())
		{
			return;
		}

		for (size_t i = 0; i < inOutGeometry.size() - 1; ++i)
		{
			MergedGeometry& firstGeometry = inOutGeometry[i];
			for (size_t j = i + 1; j < inOutGeometry.size(); ++j)
			{
				MergedGeometry& secondGeometry = inOutGeometry[j];
				if (AreShapesIntersect(firstGeometry, secondGeometry))
				{
					std::vector<SimpleBorder> newShape = ShapeOperations::GetUnion(firstGeometry.borders, secondGeometry.borders);
					ShapeOperations::OptimizeShape(newShape);

					// save the new geometry to the position of the first figure
					firstGeometry.borders = std::move(newShape);
					// remove the second figure
					inOutGeometry.erase(inOutGeometry.begin() + j);
					// retry all collision tests with the first figure
					--i;
					break;
				}
			}
		}
	}

	static void updateAABBsX(BoundingBox& box, float x)
	{
		if (x < box.minX) { box.minX = x; }
		if (x > box.maxX) { box.maxX = x; }
	}

	static void updateAABBsY(BoundingBox& box, float y)
	{
		if (y < box.minY) { box.minY = y; }
		if (y > box.maxY) { box.maxY = y; }
	}

	static void updateAABBFromBorder(BoundingBox& aabb, const SimpleBorder& border)
	{
		updateAABBsX(aabb, border.a.x);
		updateAABBsY(aabb, border.a.y);
		updateAABBsX(aabb, border.b.x);
		updateAABBsY(aabb, border.b.y);
	}

	MergedGeometry::MergedGeometry(const std::vector<Border>& inBorders, Vector2D location)
	{
		borders.reserve(inBorders.size());
		for (const Border& border : inBorders)
		{
			borders.emplace_back(border.getA() + location, border.getB() + location);
			updateAABBFromBorder(aabb, borders.back());
		}
	}

	MergedGeometry::MergedGeometry(const std::vector<SimpleBorder>& simpleBorders)
		: borders(simpleBorders)
	{
		for (const SimpleBorder& border : borders)
		{
			updateAABBFromBorder(aabb, border);
		}
	}

	void SplitIntoConvexShapes(Shape geometry, std::vector<std::vector<Vector2D>>& newShapes)
	{
		std::vector<size_t> shapes = ShapeOperations::SortBorders(geometry);
		shapes.push_back(geometry.size());

		std::vector<TPPLPoly> polygons;

		for (size_t i = 0; i < shapes.size() - 1; ++i)
		{
			const size_t pointsCount = shapes[i + 1] - shapes[i];

			TPPLPoly ttplPolygon;
			ttplPolygon.Init(pointsCount);

			const size_t startIdx = shapes[i];
			const size_t endIdx = shapes[i + 1];
			float areaSum = 0.0f;
			for (size_t idx = startIdx; idx < endIdx; ++idx)
			{
				const SimpleBorder& border = geometry[idx];
				size_t pointIdx = idx - startIdx;
				ttplPolygon[pointIdx].x = border.a.x;
				ttplPolygon[pointIdx].y = border.a.y;
				areaSum += (border.b.x - border.a.x) * (border.b.y + border.a.y);
			}
			ttplPolygon.SetHole(areaSum > 0.0f);
			polygons.push_back(std::move(ttplPolygon));
		}

		TPPLPartition pp;
		std::vector<TPPLPoly> resultPolygons;
		if (polygons.size() > 1)
		{
			// if with holes
			pp.ConvexPartition_HM(&polygons, &resultPolygons);
		}
		else
		{
			// if without holes
			pp.ConvexPartition_OPT(&polygons[0], &resultPolygons);
		}

		newShapes.reserve(resultPolygons.size());
		for (auto& polygon : resultPolygons)
		{
			newShapes.emplace_back(polygon.GetNumPoints());
			for (size_t i = 0, iSize = polygon.GetNumPoints(); i < iSize; ++i)
			{
				const TPPLPoint& point = polygon[i];
				newShapes.back()[i] = {point.x, point.y};
			}
		}
	}

	std::vector<size_t> SortBorders(Shape& inOutShape)
	{
		struct AngledBorder
		{
			SimpleBorder coords;
			float angleA;
		};

		const float oneBorderFraction = 1.0f / static_cast<float>(inOutShape.size());
		Vector2D middlePos = std::accumulate(inOutShape.begin(), inOutShape.end(), ZERO_VECTOR,
			[oneBorderFraction](Vector2D accumulatedValue, const SimpleBorder& border) -> Vector2D
			{
				return accumulatedValue + border.a * oneBorderFraction;
			}
		);

		std::vector<AngledBorder> sortedBorders(inOutShape.size());

		std::transform(
			inOutShape.begin(),
			inOutShape.end(),
			sortedBorders.begin(),
			[middlePos](const SimpleBorder& border)
			{
				return AngledBorder{
					border,
					(border.a - middlePos).rotation().getValue()
				};
			}
		);
		inOutShape.clear();

		auto lessPointAngle = [](const AngledBorder& a, const AngledBorder& b) -> bool
		{
			return a.angleA < b.angleA;
		};

		std::ranges::sort(sortedBorders, lessPointAngle);

		auto moveSortedBorders = [](std::vector<SimpleBorder>& inOutResult, std::vector<AngledBorder>& inOutSource, size_t sourceIndex)
		{
			inOutResult.push_back(inOutSource[sourceIndex].coords);
			inOutSource.erase(inOutSource.begin() + sourceIndex);
		};

		std::vector<size_t> shapeStartIndexes;
		while (!sortedBorders.empty())
		{
			shapeStartIndexes.push_back(inOutShape.size());
			moveSortedBorders(inOutShape, sortedBorders, 0);
			size_t i = 0;
			// this can be optimized
			while (true)
			{
				if (inOutShape.back().b.isNearlyEqualTo(sortedBorders[i].coords.a))
				{
					moveSortedBorders(inOutShape, sortedBorders, i);
					if (inOutShape.back().b.isNearlyEqualTo(inOutShape[shapeStartIndexes.back()].a))
					{
						break;
					}

					if ALMOST_NEVER(sortedBorders.empty()) {
						ReportError("sortedBorders should not be empty here, the shape was invalid");
						break;
					}
				}
				else
				{
					++i;
				}

				if (i >= sortedBorders.size()) { i = 0; }
			}
		}

		return shapeStartIndexes;
	}

	void ExtendInPlace(Shape& inOutShape, float radius)
	{
		float onePointFraction = 1.0f / inOutShape.size();
		Vector2D center(ZERO_VECTOR);
		for (const SimpleBorder& border : inOutShape)
		{
			center += border.a * onePointFraction;
		}

		std::ranges::for_each(inOutShape,
			[center, radius](SimpleBorder& border){
				border.a -= (center - border.a).unit() * radius;
				border.b -= (center - border.b).unit() * radius;
			}
		);
		// ToDo: actual extrude
	}
}

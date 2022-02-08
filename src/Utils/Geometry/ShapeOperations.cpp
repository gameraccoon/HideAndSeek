#include "Base/precomp.h"

#include "Utils/Geometry/ShapeOperations.h"

#include <array>
#include <algorithm>
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

	size_t addIntersectionPoint(Vector2DKey<> intersectionPoint, std::unordered_set<Vector2DKey<>>& intersectionPointsSet, std::vector<Vector2DKey<>>& intersectionPoints)
	{
		auto insertionResult = intersectionPointsSet.emplace(intersectionPoint);

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
		bool isCheckInverted = false;

		Vector2D firstPoint;
		if (border1.a.isNearlyEqualTo(intersectionPoint))
		{
			firstPoint = border1.b;
			isCheckInverted = !isCheckInverted;
		}
		else
		{
			firstPoint = border1.a;
		}

		Vector2D secondPoint;
		if (border2.b.isNearlyEqualTo(intersectionPoint))
		{
			secondPoint = border2.a;
			isCheckInverted = !isCheckInverted;
		}
		else
		{
			secondPoint = border2.b;
		}

		float signedArea = Collide::SignedArea(firstPoint, secondPoint, intersectionPoint);

		return isCheckInverted ? (signedArea < 0) : (signedArea > 0);
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

	static std::array<float, 4> InverseMatrix(std::array<float, 4> matrix)
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
		// ToDo: add a comment about the meaning of shouldInverse
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
			std::unordered_set<Vector2DKey<>>& intersectionPointsSet,
			std::vector<Vector2DKey<>>& intersectionPoints,
			std::vector<BorderIntersection>& borderAIntersections,
			std::vector<BorderIntersection>& borderBIntersections)
	{
		// project borderB into coordinate system based on a borderA
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
				size_t pointIdx = addIntersectionPoint(Vector2DKey<>(borderB.a), intersectionPointsSet, intersectionPoints);

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
				size_t pointIdx = addIntersectionPoint(Vector2DKey<>(borderB.b), intersectionPointsSet, intersectionPoints);

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
				size_t pointIdx = addIntersectionPoint(Vector2DKey<>(borderA.a), intersectionPointsSet, intersectionPoints);

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
				size_t pointIdx = addIntersectionPoint(Vector2DKey<>(borderA.b), intersectionPointsSet, intersectionPoints);

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
		using VectorKey = Vector2DKey<>;
		using KeyPair = std::pair<VectorKey, VectorKey>;
		using IndexPair = std::pair<size_t, size_t>;

		std::vector<VectorKey> shapePoints;
		std::unordered_map<VectorKey, size_t> shapePointsMap;
		std::vector<IndexPair> shape1Borders;
		shape1Borders.reserve(shape1.size());
		std::vector<IndexPair> shape2Borders;
		shape2Borders.reserve(shape2.size());

		auto getKeyIndex = [&shapePoints, &shapePointsMap](const Vector2D pos) -> size_t
		{
			const VectorKey key(pos);
			auto it = shapePointsMap.find(key);
			if (it != shapePointsMap.end())
			{
				return it->second;
			}

			shapePoints.push_back(key);
			shapePointsMap.emplace(key, shapePoints.size() - 1);
			return shapePoints.size() - 1;
		};

		for (const SimpleBorder& border : shape1)
		{
			shape1Borders.emplace_back(getKeyIndex(border.a), getKeyIndex(border.b));
		}
		for (const SimpleBorder& border : shape2)
		{
			shape2Borders.emplace_back(getKeyIndex(border.a), getKeyIndex(border.b));
		}

		const size_t shape1BordersCount = shape1.size();
		std::vector<VectorKey> intersectionPoints;
		std::unordered_set<VectorKey> intersectionPointsSet;
		std::unordered_map<size_t, std::vector<BorderIntersection>> borderIntersections;

		// find all intersections
		for (size_t i = 0; i < shape1Borders.size(); ++i)
		{
			IndexPair borderIdxs = shape1Borders[i];
			SimpleBorder border{shapePoints[borderIdxs.first].calcRoundedValue(), shapePoints[borderIdxs.second].calcRoundedValue()};
			// create items even for the borders without intersections
			std::vector<BorderIntersection>& intersections = borderIntersections[i];
			for (size_t j = 0; j < shape2.size(); ++j)
			{
				IndexPair otherBorderIdxs = shape2Borders[j];
				SimpleBorder otherBorder{shapePoints[otherBorderIdxs.first].calcRoundedValue(), shapePoints[otherBorderIdxs.second].calcRoundedValue()};
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

					size_t pointIdx = addIntersectionPoint(VectorKey(intersectionPoint), intersectionPointsSet, intersectionPoints);
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

		// create empty items for the borders without intersections
		for (size_t j = 0; j < shape2.size(); ++j)
		{
			borderIntersections.try_emplace(shape1BordersCount + j);
		}

		std::vector<KeyPair> fracturedBorders;
		std::unordered_map<VectorKey, std::vector<size_t>> borderConnections;

		// split borders into smaller parts
		for (const auto& [borderIndex, intersections] : borderIntersections)
		{
			const SimpleBorder& border = (borderIndex < shape1BordersCount) ? shape1[borderIndex] : shape2[borderIndex - shape1BordersCount];

			std::vector<BorderPoint> borderPoints({
				BorderPoint(border.a),
				BorderPoint(border.b)
			});
			std::vector<float> borderPointFractions({0.0f, (border.a - border.b).size()});
			for (auto [intersectionIdx, cutDirection] : intersections)
			{
				const VectorKey& intersectionPoint = intersectionPoints[intersectionIdx];
				const float intersectionFraction = (border.a - intersectionPoint.value).size();
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
					borderPoints.emplace(borderPoints.begin() + static_cast<ptrdiff_t>(newPosition), intersectionPoint.value, cutDirection);
				}
			}

			AssertFatal(!borderPoints.empty(), "borderPoints should always contain at least two elements");
			for (size_t i = 1; i < borderPoints.size(); ++i)
			{
				fracturedBorders.emplace_back(VectorKey(borderPoints[i - 1].pos), VectorKey(borderPoints[i].pos));

				if (!IsSideOutside(borderPoints[i - 1].cutDirection.secondSidePosition)
					&& !IsSideOutside(borderPoints[i].cutDirection.firstSidePosition))
				{
					borderConnections[fracturedBorders.back().first].push_back(fracturedBorders.size() - 1);
				}
			}
		}

		// find all chains of borders that should be eliminated
		std::vector<size_t> bordersToRemove;
		for (const VectorKey& intersectionPoint : intersectionPoints)
		{
			const std::vector<size_t>& connections = borderConnections[intersectionPoint];
			for (size_t startBorderIdx : connections)
			{
				if (fracturedBorders[startBorderIdx].first == intersectionPoint)
				{
					size_t borderIdx = startBorderIdx;
					while(true) // exit in the middle
					{
						bordersToRemove.push_back(borderIdx);
						VectorKey nextPoint(fracturedBorders[borderIdx].second);
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
			fracturedBorders.erase(fracturedBorders.begin() + static_cast<ptrdiff_t>(index));
		}

		// remove any duplicated borders
		// ToDo: probably need to correct the logic above to normally eliminate such borders
		// instead of doing this NlogN overcomplicated stuff to fight a super-rare case
		std::ranges::sort(fracturedBorders);
		auto last = std::unique(fracturedBorders.begin(), fracturedBorders.end());
		fracturedBorders.erase(last, fracturedBorders.end());

		std::vector<SimpleBorder> resultingBorders;
		resultingBorders.resize(fracturedBorders.size());

		std::ranges::transform(
			fracturedBorders.begin(),
			fracturedBorders.end(),
			resultingBorders.begin(),
			[](const KeyPair& k){
				return SimpleBorder{ k.first.value, k.second.value };
			}
		);

		return resultingBorders;
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

		using VectorKey = Vector2DKey<>;

		// collect neighboring borders
		std::unordered_map<VectorKey, std::vector<BorderInfo>> points;
		for (size_t i = 0, iSize = inOutShape.size(); i < iSize; ++i)
		{
			const SimpleBorder& border = inOutShape[i];
			points[VectorKey(border.a)].emplace_back(border.b, i, true);
			points[VectorKey(border.b)].emplace_back(border.a, i, false);
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
						const Vector2D vec1 = pos.value - borders[i].secondBorderPoint;
						const Vector2D vec2 = pos.value - borders[j].secondBorderPoint;
						const float dotProduct = Vector2D::DotProduct(vec1.unit(), vec2.unit());
						if (dotProduct < -0.98f)
						{
							const size_t finalBorderIdx = borders[i].borderIndex;
							const size_t borderIdxToRemove = borders[j].borderIndex;
							const Vector2D notChangedBorderPoint = borders[i].secondBorderPoint;
							const Vector2D movedBorderPoint = borders[j].secondBorderPoint;

							{
								Vector2D secondFinalBorderPoint;
								// replace border i with the merged border in the final shape
								SimpleBorder& finalBorder = inOutShape[finalBorderIdx];
								if (finalBorder.a.isNearlyEqualTo(pos.value, 0.001f))
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
								auto pointsIt = points.find(VectorKey(secondFinalBorderPoint));
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
								const Vector2D anotherIntersectionPoint = (borderToRemove.a.isNearlyEqualTo(pos.value, 0.001f)) ? borderToRemove.b : borderToRemove.a;
								auto pointsIt = points.find(VectorKey(anotherIntersectionPoint));
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
			inOutShape.erase(inOutShape.begin() + static_cast<int>(index));
		}
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

		for (size_t i = 0; i + 1 < inOutGeometry.size(); ++i)
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
					firstGeometry.updateAABB();
					// remove the second figure
					inOutGeometry.erase(inOutGeometry.begin() + static_cast<ptrdiff_t>(j));
					// retry all collision tests with the first figure
					--i;
				}
			}
		}
	}

	static void UpdateAABBsX(BoundingBox& box, float x)
	{
		if (x < box.minX) { box.minX = x; }
		if (x > box.maxX) { box.maxX = x; }
	}

	static void UpdateAABBsY(BoundingBox& box, float y)
	{
		if (y < box.minY) { box.minY = y; }
		if (y > box.maxY) { box.maxY = y; }
	}

	static void UpdateAABBFromBorder(BoundingBox& aabb, const SimpleBorder& border)
	{
		UpdateAABBsX(aabb, border.a.x);
		UpdateAABBsY(aabb, border.a.y);
		UpdateAABBsX(aabb, border.b.x);
		UpdateAABBsY(aabb, border.b.y);
	}

	MergedGeometry::MergedGeometry(const std::vector<Border>& inBorders, Vector2D location)
	{
		borders.reserve(inBorders.size());
		for (const Border& border : inBorders)
		{
			borders.emplace_back(border.getA() + location, border.getB() + location);
			UpdateAABBFromBorder(aabb, borders.back());
		}
	}

	MergedGeometry::MergedGeometry(const std::vector<SimpleBorder>& simpleBorders)
		: borders(simpleBorders)
	{
		updateAABB();
	}

	MergedGeometry::MergedGeometry(std::vector<SimpleBorder>&& simpleBorders)
		: borders(std::move(simpleBorders))
	{
		updateAABB();
	}

	void MergedGeometry::updateAABB()
	{
		for (const SimpleBorder& border : borders)
		{
			UpdateAABBFromBorder(aabb, border);
		}
	}

	void SplitIntoConvexShapes(std::vector<std::vector<Vector2D>>& inOutShapes, Shape geometry)
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

		for (auto& polygon : resultPolygons)
		{
			inOutShapes.emplace_back(polygon.GetNumPoints());
			for (size_t i = 0, iSize = polygon.GetNumPoints(); i < iSize; ++i)
			{
				const TPPLPoint& point = polygon[i];
				inOutShapes.back()[i] = {point.x, point.y};
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
			inOutSource.erase(inOutSource.begin() + static_cast<ptrdiff_t>(sourceIndex));
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

	void ExtendGeometry(Shape& outResultingShape, const std::vector<Vector2D>& geometry, float radius)
	{
		outResultingShape.clear();

		std::vector<Vector2D> points(geometry.size());

		auto getNeighboringPoints = [size = points.size()](size_t i) -> std::pair<size_t, size_t>
		{
			return { (i > 0) ? i - 1 : size - 1, (i < size - 1) ? i + 1 : 0 };
		};

		for (size_t i = 0, iSize = points.size(); i < iSize; ++i)
		{
			auto [prev, next] = getNeighboringPoints(i);
			Vector2D direction = ((geometry[i] - geometry[prev]).normal() + (geometry[next] - geometry[i]).normal()).unit();
			points[i] = geometry[i] + direction * radius;
		}

		FOR_EACH_BORDER(points.size(), {
			outResultingShape.emplace_back(points[i], points[j]);
		});
	}
}

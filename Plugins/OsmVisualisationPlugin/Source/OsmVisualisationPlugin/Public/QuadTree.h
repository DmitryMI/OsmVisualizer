#pragma once

#include "CoreMinimal.h"
#include "Templates/UniquePtr.h"

struct FLatLonBoundingBox
{
public:
	FVector2d centerLatLon;

	FVector2d angleHalfSize;

	FLatLonBoundingBox()
	{

	}

	FLatLonBoundingBox(const FVector2d& center, FVector2d angleHalfSize)
	{
		this->centerLatLon = center;
		this->angleHalfSize = angleHalfSize;
	}

	bool Contains(const FVector2d& point) const
	{
		double dLat = FMath::Abs(FMath::FindDeltaAngleDegrees(centerLatLon.X, point.X));
		double dLon = FMath::Abs(FMath::FindDeltaAngleDegrees(centerLatLon.Y, point.Y));
		return dLat < angleHalfSize.X && dLon < angleHalfSize.Y;
	}

	bool Intersects(const FLatLonBoundingBox& box) const
	{
		double dLat = FMath::Abs(FMath::FindDeltaAngleDegrees(centerLatLon.X, box.centerLatLon.X));
		double dLon = FMath::Abs(FMath::FindDeltaAngleDegrees(centerLatLon.Y, box.centerLatLon.Y));

		bool latIntersects = dLat < (centerLatLon.X + box.centerLatLon.X);
		bool lonIntersects = dLon < (centerLatLon.Y + box.centerLatLon.Y);
		return latIntersects && lonIntersects;
	}

	FVector2d GetNorthWestPoint() const
	{
		return FVector2d(centerLatLon.X + angleHalfSize.X, centerLatLon.Y - angleHalfSize.Y);
	}

	FVector2d GetNorthEastPoint() const
	{
		return FVector2d(centerLatLon.X + angleHalfSize.X, centerLatLon.Y + angleHalfSize.Y);
	}

	FVector2d GetSouthWestPoint() const
	{
		return FVector2d(centerLatLon.X - angleHalfSize.X, centerLatLon.Y - angleHalfSize.Y);
	}

	FVector2d GetSouthEastPoint() const
	{
		return FVector2d(centerLatLon.X - angleHalfSize.X, centerLatLon.Y + angleHalfSize.Y);
	}

	/// <summary>
	/// Keeps Lat in range [-90, 90] and Lon in range [-180, 180]
	/// </summary>
	void Normalize()
	{
		this->centerLatLon.X = FMath::UnwindDegrees(this->centerLatLon.X);		
		
		if (centerLatLon.X > 90)
		{
			centerLatLon.X = 180 - centerLatLon.X;
			this->centerLatLon.Y += 180;
		}
		else if (centerLatLon.X < -90)
		{
			centerLatLon.X = -180 - centerLatLon.X;
			this->centerLatLon.Y += 180;
		}

		this->centerLatLon.Y = FMath::UnwindDegrees(this->centerLatLon.Y);

		if (centerLatLon.Y > 180)
		{
			centerLatLon.Y -= 360;
		}
		else if (centerLatLon.Y < -180)
		{
			centerLatLon.Y += 360;
		}
	}

	void Subdivide(
		FLatLonBoundingBox& northWest,
		FLatLonBoundingBox& northEast,
		FLatLonBoundingBox& southWest,
		FLatLonBoundingBox& southEast
	) const
	{
		northWest.centerLatLon.X = centerLatLon.X + angleHalfSize.X / 4;
		northWest.centerLatLon.Y = centerLatLon.Y - angleHalfSize.Y / 4;
		northWest.angleHalfSize = angleHalfSize / 2;
		northWest.Normalize();

		northEast.centerLatLon.X = centerLatLon.X + angleHalfSize.X / 4;
		northEast.centerLatLon.Y = centerLatLon.Y + angleHalfSize.Y / 4;
		northEast.angleHalfSize = angleHalfSize / 2;
		northEast.Normalize();

		southWest.centerLatLon.X = centerLatLon.X - angleHalfSize.X / 4;
		southWest.centerLatLon.Y = centerLatLon.Y - angleHalfSize.Y / 4;
		southWest.angleHalfSize = angleHalfSize / 2;
		southWest.Normalize();

		southEast.centerLatLon.X = centerLatLon.X - angleHalfSize.X / 4;
		southEast.centerLatLon.Y = centerLatLon.Y + angleHalfSize.Y / 4;
		southEast.angleHalfSize = angleHalfSize / 2;
		southEast.Normalize();
	}
};

template<typename PointData>
class FQuadTree
{
private:
	int nodeCapacity = 128;
	FLatLonBoundingBox boundary;

	TArray<TPair<FVector2d, PointData>> points;

	TUniquePtr<FQuadTree<PointData>> northWest;
	TUniquePtr<FQuadTree<PointData>> northEast;
	TUniquePtr<FQuadTree<PointData>> southWest;
	TUniquePtr<FQuadTree<PointData>> southEast;

	void Subdivide()
	{
		FLatLonBoundingBox northWestBoundary;
		FLatLonBoundingBox northEastBoundary;
		FLatLonBoundingBox southWestBoundary;
		FLatLonBoundingBox southEastBoundary;
		boundary.Subdivide(northWestBoundary, northEastBoundary, southWestBoundary, southEastBoundary);
		northWest.Reset(new FQuadTree<PointData>(northWestBoundary, nodeCapacity));
		northEast.Reset(new FQuadTree<PointData>(northEastBoundary, nodeCapacity));
		southWest.Reset(new FQuadTree<PointData>(southWestBoundary, nodeCapacity));
		southEast.Reset(new FQuadTree<PointData>(southEastBoundary, nodeCapacity));
	}


public:
	FQuadTree()
	{

	}

	FQuadTree(const FLatLonBoundingBox& box)
	{
		this->boundary = box;
	}

	FQuadTree(const FLatLonBoundingBox& box, int nodeCapacity)
	{
		this->boundary = box;
		this->nodeCapacity = nodeCapacity;
	}

	TArray<TPair<FVector2d, PointData>> Query(const FLatLonBoundingBox& range)
	{
		TArray<TPair<FVector2d, PointData>> result;
		Query(box, result);
		return result;
	}

	void Query(const FLatLonBoundingBox& range, TArray<TPair<FVector2d, PointData>>& OutResult) const
	{
		if (!boundary.Intersects(range))
		{
			return;
		}

		for (auto& point : points)
		{
			if (range.Contains(point.Key))
			{
				OutResult.Add(point);
			}
		}

		if (northWest == nullptr)
		{
			return;
		}

		northWest->Query(range, OutResult);
		northEast->Query(range, OutResult);
		southWest->Query(range, OutResult);
		southEast->Query(range, OutResult);
	}

	bool Insert(const TPair<FVector2d, PointData>& point)
	{
		if (!boundary.Contains(point.Key))
		{
			return false;
		}

		if (points.Num() < nodeCapacity && northWest == nullptr)
		{
			points.Add(point);
			return true;
		}

		if (northWest == nullptr)
		{
			Subdivide();
		}

		if (northWest.Get()->Insert(point))
		{
			return true;
		}
		if (northEast.Get()->Insert(point))
		{
			return true;
		}
		if (southWest.Get()->Insert(point))
		{
			return true;
		}
		if (southEast.Get()->Insert(point))
		{
			return true;
		}

		// Should never happen
		return false;
	}

	bool Insert(FVector2d latLon, PointData data)
	{
		TPair<FVector2d, PointData> point(latLon, data);
		return Insert(point);
	}

	TArray<FQuadTree*> GetSubtrees() const
	{
		TArray<FQuadTree*> result;
		GetSubtrees(result);
		return result;
	}

	FLatLonBoundingBox GetLatLonBoundingBox() const
	{
		return boundary;
	}

	void GetSubtrees(TArray<FQuadTree*>& OutResult) const
	{
		if (northWest)
		{
			OutResult.Add(northWest.Get());
			OutResult.Add(northEast.Get());
			OutResult.Add(southWest.Get());
			OutResult.Add(southEast.Get());
		}		
	}

	const TArray<TPair<FVector2d, PointData>>& GetPoints() const
	{
		return points;
	}
};
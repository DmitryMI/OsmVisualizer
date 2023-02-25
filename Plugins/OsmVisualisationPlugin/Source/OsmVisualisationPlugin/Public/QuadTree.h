#pragma once

#include "CoreMinimal.h"
#include "LatLonBoundingBox.h"
#include "Templates/UniquePtr.h"

template<typename PointData>
class FQuadTree
{
private:
	int nodeCapacity = 128;
	FLatLonBoundingBox boundary;

	TArray<TPair<FVector2D, PointData>> points;

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

	TArray<TPair<FVector2D, PointData>> Query(const FLatLonBoundingBox& range)
	{
		TArray<TPair<FVector2D, PointData>> result;
		Query(box, result);
		return result;
	}

	void Query(const FLatLonBoundingBox& range, TArray<TPair<FVector2D, PointData>>& OutResult) const
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

	bool Insert(const TPair<FVector2D, PointData>& point)
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

	bool Insert(FVector2D latLon, PointData data)
	{
		TPair<FVector2D, PointData> point(latLon, data);
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

	const TArray<TPair<FVector2D, PointData>>& GetPoints() const
	{
		return points;
	}
};
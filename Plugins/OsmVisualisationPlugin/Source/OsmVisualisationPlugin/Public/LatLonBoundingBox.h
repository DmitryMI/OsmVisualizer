#pragma once

#include "LatLonBoundingBox.generated.h"

USTRUCT(BlueprintType)
struct FLatLonBoundingBox
{
	GENERATED_BODY()
public:
	FVector2D centerLatLon;

	FVector2D angleHalfSize;

	FLatLonBoundingBox()
	{

	}

	FLatLonBoundingBox(const FVector2D& center, FVector2D angleHalfSize)
	{
		this->centerLatLon = center;
		this->angleHalfSize = angleHalfSize;
	}

	bool Contains(const FVector2D& point) const
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

	FVector2D GetNorthWestPoint() const
	{
		return FVector2D(centerLatLon.X + angleHalfSize.X, centerLatLon.Y - angleHalfSize.Y);
	}

	FVector2D GetNorthEastPoint() const
	{
		return FVector2D(centerLatLon.X + angleHalfSize.X, centerLatLon.Y + angleHalfSize.Y);
	}

	FVector2D GetSouthWestPoint() const
	{
		return FVector2D(centerLatLon.X - angleHalfSize.X, centerLatLon.Y - angleHalfSize.Y);
	}

	FVector2D GetSouthEastPoint() const
	{
		return FVector2D(centerLatLon.X - angleHalfSize.X, centerLatLon.Y + angleHalfSize.Y);
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
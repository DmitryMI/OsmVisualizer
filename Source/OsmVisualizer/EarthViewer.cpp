// Fill out your copyright notice in the Description page of Project Settings.


#include "EarthViewer.h"
#include "Intersection/IntersectionUtil.h"
#include "DrawDebugHelpers.h"

FVector AEarthViewer::ConvertSphericalCoordinatesDeg(const FVector2D& latLon, double radius) const
{
	// x = r * sin(Theta) * cos(Phi)
	// y = r * sin(Theta) * sin(Phi)
	// z = r * cos(Theta)

	// Theta - Longtiture
	// Phi - Latitude

	double theta = FMath::DegreesToRadians(latLon.X);
	double phi = FMath::DegreesToRadians(latLon.Y);

	double x = radius * FMath::Sin(theta) * FMath::Cos(phi);
	double y = radius * FMath::Sin(theta) * FMath::Sin(phi);
	double z = radius * FMath::Cos(theta);
	return FVector(x, y, z);
}

FVector2D AEarthViewer::ConvertToSphericalCoordinatesDeg(const FVector& point, double& outRadius) const
{
	outRadius = point.Size();
	double theta = FMath::Acos(point.Z / outRadius);
	double phi = FMath::Sign(point.Y) * FMath::Acos(point.X / point.Size2D());

	return FVector2D(FMath::RadiansToDegrees(theta), FMath::RadiansToDegrees(phi));
}

void AEarthViewer::CreateProceduralMesh(const FVector2D& latLonMin, const FVector2D& latLonMax)
{

	TArray<FVector> vertices;
	TArray<int32> triangles;

	double stepLon = (latLonMax.Y - latLonMin.Y) / (meshGridSize.X - 1);
	double stepLat = (latLonMax.X - latLonMin.X) / (meshGridSize.Y - 1);

	double currentLon = latLonMin.Y;

	for (int x = 0; x < meshGridSize.X; x++)
	{
		double currentLat = latLonMin.X;
		for (int y = 0; y < meshGridSize.Y; y++)
		{
			FVector coord = ConvertSphericalCoordinatesDeg(FVector2D(currentLat, currentLon), planetRadius);

#if WITH_EDITOR
			FRotator rotation = proceduralMeshComponent->GetRelativeRotation();
			FVector coordRotated = rotation.RotateVector(coord);
			DrawDebugPoint(GetWorld(), coordRotated, 10.0, FColor::Blue);
#endif
			vertices.Add(coord);
			currentLat += stepLat;
		}
		
		currentLon += stepLon;
	}

	for (int x = 0; x < meshGridSize.X - 1; x++)
	{
		for (int y = 0; y < meshGridSize.Y - 1; y++)
		{
			triangles.Add((y + 1) * meshGridSize.X + x + 1);
			triangles.Add(y * meshGridSize.X + x + 1);
			triangles.Add(y * meshGridSize.X + x);
			
			triangles.Add((y + 1) * meshGridSize.X + x + 1);
			triangles.Add(y * meshGridSize.X + x);
			triangles.Add((y + 1) * meshGridSize.X + x);
		}
	}

	if (proceduralMeshComponent)
	{
		proceduralMeshComponent->CreateMeshSection_LinearColor(
			0,
			vertices,
			triangles,
			TArray<FVector>(),
			TArray<FVector2D>(),
			TArray<FLinearColor>(),
			TArray<FProcMeshTangent>(),
			false
		);
	}
	

	// DrawDebugMesh(GetWorld(), vertices, triangles, FColor::Green);
}

// Sets default values
AEarthViewer::AEarthViewer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEarthViewer::BeginPlay()
{
	Super::BeginPlay();
	
	proceduralMeshComponent = GetComponentByClass<UProceduralMeshComponent>();
	ensure(proceduralMeshComponent);
}

// Called every frame
void AEarthViewer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector sphereCenter = GetActorLocation();

	DrawDebugSphere(GetWorld(), sphereCenter, planetRadius, 128, FColor::Red);


	APlayerController* localPlayer = GetWorld()->GetFirstPlayerController();
	check(localPlayer);

	int32 screenWidth, screenHeight;
	localPlayer->GetViewportSize(screenWidth, screenHeight);

	FVector topRightPos, topRightDir;
	localPlayer->DeprojectScreenPositionToWorld(0, 0, topRightPos, topRightDir);
	
	FVector topLeftPos, topLeftDir;
	localPlayer->DeprojectScreenPositionToWorld(screenWidth, 0, topLeftPos, topLeftDir);
	
	FVector botRightPos, botRightDir;
	localPlayer->DeprojectScreenPositionToWorld(0, screenHeight, botRightPos, botRightDir);

	FVector botLeftPos, botLeftDir;
	localPlayer->DeprojectScreenPositionToWorld(screenWidth, screenHeight, botLeftPos, botLeftDir);

	IntersectionUtil::FLinearIntersection topRightIntersection;
	IntersectionUtil::LineSphereIntersection(topRightPos, topRightDir, sphereCenter, planetRadius, topRightIntersection);
	
	IntersectionUtil::FLinearIntersection topLeftIntersection;
	IntersectionUtil::LineSphereIntersection(topLeftPos, topLeftDir, sphereCenter, planetRadius, topLeftIntersection);
	
	IntersectionUtil::FLinearIntersection botLeftIntersection;
	IntersectionUtil::LineSphereIntersection(botLeftPos, botLeftDir, sphereCenter, planetRadius, botLeftIntersection);
	
	IntersectionUtil::FLinearIntersection botRightIntersection;
	IntersectionUtil::LineSphereIntersection(botRightPos, botRightDir, sphereCenter, planetRadius, botRightIntersection);

	if (!(topRightIntersection.intersects && topLeftIntersection.intersects && botLeftIntersection.intersects && botRightIntersection.intersects))
	{
		return;
	}

	FVector topRight = topRightPos + topRightDir * topRightIntersection.parameter.Min;
	FVector topLeft = topLeftPos + topLeftDir * topLeftIntersection.parameter.Min;
	FVector botRight = botRightPos + botRightDir * botRightIntersection.parameter.Min;
	FVector botLeft = botLeftPos + botLeftDir * botLeftIntersection.parameter.Min;

	DrawDebugPoint(GetWorld(), topRight, 12, FColor::Green);
	DrawDebugPoint(GetWorld(), topLeft, 12, FColor::Green);
	DrawDebugPoint(GetWorld(), botRight, 12, FColor::Green);
	DrawDebugPoint(GetWorld(), botLeft, 12, FColor::Green);

	double radiusPlusHeight;
	FVector2D topRightLatLon = ConvertToSphericalCoordinatesDeg(topRight, radiusPlusHeight);
	FVector2D topLeftLatLon = ConvertToSphericalCoordinatesDeg(topLeft, radiusPlusHeight);
	FVector2D botRightLatLon = ConvertToSphericalCoordinatesDeg(botRight, radiusPlusHeight);
	FVector2D botLeftLatLon = ConvertToSphericalCoordinatesDeg(botLeft, radiusPlusHeight);

	UE_LOG(LogTemp, Display, TEXT("%s\t%s"), *(topRightLatLon.ToString()), *(topLeftLatLon.ToString()));
	UE_LOG(LogTemp, Display, TEXT("%s\t%s"), *(botRightLatLon.ToString()), *(botLeftLatLon.ToString()));

	double latDelta = FMath::FindDeltaAngleDegrees(topRightLatLon.X, botRightLatLon.X);
	double lonDelta = FMath::FindDeltaAngleDegrees(topLeftLatLon.Y, topRightLatLon.Y);

	//double latDelta = topRightLatLon.X - botRightLatLon.X;
	//double lonDelta = topLeftLatLon.Y - topRightLatLon.Y;

	UE_LOG(LogTemp, Display, TEXT("Angular size: (%3.2f, %3.2f). Radius: %3.2f"), latDelta, lonDelta, radiusPlusHeight);

	if (latDelta <= 0 || lonDelta <= 0)
	{
		return;
	}

	//UE_LOG(LogTemp, Display, TEXT("Angular size: (%3.2f, %3.2f)"), latDelta, lonDelta);
	
	FVector viewportCenter = (topRight + topLeft + botRight + botLeft) / 4;
	FVector viewportVec = viewportCenter - sphereCenter;
	FRotator rotationToViewport = viewportVec.Rotation();
	proceduralMeshComponent->SetWorldRotation(rotationToViewport);

	double latMin = 90 - latDelta / 2;
	double latMax = 90 + latDelta / 2;
	double lonMin = -lonDelta / 2;
	double lonMax = lonDelta / 2;
	CreateProceduralMesh(FVector2D(latMin, lonMin), FVector2D(latMax, lonMax));
	// CreateProceduralMesh(topRightLatLon, botLeftLatLon);
}


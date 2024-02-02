// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "ProceduralMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "EarthViewer.generated.h"

UCLASS()
class OSMVISUALIZER_API AEarthViewer : public AActor
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere)
	UProceduralMeshComponent* proceduralMeshComponent;

	UPROPERTY(EditAnywhere)
	double planetRadius = 1000.0;

	UPROPERTY(EditAnywhere)
	FIntVector2 meshGridSize = FIntVector2(10, 10);

private:

	FVector ConvertSphericalCoordinatesDeg(const FVector2D& latLon, double radius) const;

	FVector2D ConvertToSphericalCoordinatesDeg(const FVector& point, double& outRadius) const;

	void CreateProceduralMesh(const FVector2D& latLonMin, const FVector2D& latLonMax);

public:	
	// Sets default values for this actor's properties
	AEarthViewer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};

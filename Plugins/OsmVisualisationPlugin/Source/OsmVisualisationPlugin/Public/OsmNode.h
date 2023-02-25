#pragma once

#include "CoreMinimal.h"
#include "OsmNode.generated.h"

USTRUCT(BlueprintType)
struct FOsmNode
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere)
	int64 id;
	
	UPROPERTY(EditAnywhere)
	double lat;

	UPROPERTY(EditAnywhere)
	double lon;

	UPROPERTY(EditAnywhere)
	TMap<FString, FString> tags;

public:
	FVector2D GetLatLon() const
	{
		return FVector2D(lat, lon);
	}
};
#pragma once

#include "CoreMinimal.h"
#include "OsmWay.generated.h"

USTRUCT(BlueprintType)
struct FOsmWay
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere)
	int64 id;
	
	UPROPERTY(EditAnywhere)
	TArray<int64> nodeIds;
	
	UPROPERTY(EditAnywhere)
	TMap<FString, FString> tags;
};
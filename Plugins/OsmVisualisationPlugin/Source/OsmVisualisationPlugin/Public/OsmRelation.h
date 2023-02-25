#pragma once

#include "CoreMinimal.h"
#include "OsmRelationMember.h"
#include "OsmRelation.generated.h"

USTRUCT(BlueprintType)
struct FOsmRelation
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere)
	int64 id;
	
	UPROPERTY(EditAnywhere)
	TArray<FOsmRelationMember> members;
	
	UPROPERTY(EditAnywhere)
	TMap<FString, FString> tags;
};
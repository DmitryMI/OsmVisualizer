#pragma once

#include "CoreMinimal.h"
#include "OsmRelationMember.generated.h"

UENUM(BlueprintType)
enum class OsmRelationMemberType : uint8
{
	RMT_Node,
	RMT_Way,
	RMT_Relation
};

USTRUCT(BlueprintType)
struct FOsmRelationMember
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere)
	OsmRelationMemberType type;
	
	UPROPERTY(EditAnywhere)
	int64 ref;
	
	UPROPERTY(EditAnywhere)
	FString role;
};
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OsmNode.h"
#include "OsmWay.h"
#include "OsmRelation.h"
#include "Dom/JsonObject.h"
#include "JsonObjectWrapper.h"
#include "QuadTree.h"
#include "Earth.generated.h"

UCLASS()
class OSMVISUALISATIONPLUGIN_API AEarth : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly)
	double planetRealRadius = 6.3781e6 * 100;

	UPROPERTY(EditAnywhere)
	double planetVisualRadius = 10000.0;

	UPROPERTY(EditAnywhere)
	TMap<int64, FOsmNode> osmNodes;

	UPROPERTY(EditAnywhere)
	TMap<int64, FOsmWay> osmWays;

	UPROPERTY(EditAnywhere)
	TMap<int64, FOsmRelation> osmRelations;

	TUniquePtr<FQuadTree<int64>> nodeSpatialIndex;
	
public:	
	// Sets default values for this actor's properties
	AEarth();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	template<typename OsgElement>
	bool LoadTagsFromJsonArray(OsgElement& osgElement, const TSharedPtr<FJsonObject>& jsonObjectPtr)
	{
		if (jsonObjectPtr->HasField("tags"))
		{
			auto tagsObject = jsonObjectPtr->GetObjectField("tags");
			for (auto tag : tagsObject->Values)
			{
				FString tagValue;
				if (!tag.Value->TryGetString(tagValue))
				{
					UE_LOG(LogTemp, Error, TEXT("Non-string tag!"));
					return false;
				}
				osgElement.tags.Add(tag.Key, tagValue);
			}
		}

		return true;
	}

	virtual bool LoadRelationMemberFromJsonObject(const TSharedPtr<FJsonObject>& jsonObjectPtr, FOsmRelationMember& member);
	virtual bool LoadNodeFromJsonObject(const TSharedPtr<FJsonObject>& jsonObjectPtr);
	virtual bool LoadWayFromJsonObject(const TSharedPtr<FJsonObject>& jsonObjectPtr);
	virtual bool LoadRelationFromJsonObject(const TSharedPtr<FJsonObject>& jsonObjectPtr);
public:	

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	FVector ConvertSphericalCoordinatesDeg(const FVector2D& latLon, double radius) const;

	UFUNCTION(BlueprintCallable)
	FVector LatLonToRealSpace(const FVector2D& latLon) const;

	UFUNCTION(BlueprintCallable)
	FVector LatLonToRelativeSpace(const FVector2D& latLon) const;

	UFUNCTION(BlueprintCallable)
	FVector LatLonToWorldSpace(const FVector2D& latLon) const;

	UFUNCTION(BlueprintCallable)
	void ClearOsmData();

	UFUNCTION(BlueprintCallable)
	bool LoadFromJsonObject(const FJsonObjectWrapper& jsonObjectWrapper);

	UFUNCTION(BlueprintCallable)
	const TMap<int64, FOsmNode>& GetNodes();
	UFUNCTION(BlueprintCallable)
	const TMap<int64, FOsmWay>& GetWays();
	UFUNCTION(BlueprintCallable)
	const TMap<int64, FOsmRelation>& GetRelations();

	UFUNCTION(BlueprintCallable)
	void BuildSpatialIndex();

	UFUNCTION(BlueprintCallable)
	void DebugDrawGeoLine(const FVector2D& latLonFrom, const FVector2D& latLonTo, double angleStep, const FColor& color, float time) const;

	void DebugDrawQuadTreeNode(FQuadTree<int64>* quadTreeNode, double time) const;

	UFUNCTION(BlueprintCallable)
	void DebugDrawSpatialIndex(double time) const;
};

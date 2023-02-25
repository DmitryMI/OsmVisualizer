// Fill out your copyright notice in the Description page of Project Settings.


#include "Earth.h"
#include "DrawDebugHelpers.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"

// Sets default values
AEarth::AEarth()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEarth::BeginPlay()
{
	Super::BeginPlay();
	
	auto components = GetComponents();
	for (auto& component : components)
	{
		if (component->GetName() == "NiagaraBuildings")
		{
			buildingVisualizer = Cast<UNiagaraComponent>(component);
		}
	}

	ensure(buildingVisualizer);

	RenderBuildings();
}

// Called every frame
void AEarth::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector AEarth::ConvertSphericalCoordinatesDeg(const FVector2D& latLon, double radius) const
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

FVector AEarth::LatLonToRealSpace(const FVector2D& latLon) const
{
	return ConvertSphericalCoordinatesDeg(latLon, planetRealRadius);
}

FVector AEarth::LatLonToRelativeSpace(const FVector2D& latLon) const
{
	return ConvertSphericalCoordinatesDeg(latLon, planetVisualRadius);
}

FVector AEarth::LatLonToWorldSpace(const FVector2D& latLon) const
{
	FVector relative = LatLonToRelativeSpace(latLon);
	return relative + GetActorLocation();
}

void AEarth::ClearOsmData()
{
	osmNodes.Empty();
	osmWays.Empty();
	osmRelations.Empty();
}

bool AEarth::LoadRelationMemberFromJsonObject(const TSharedPtr<FJsonObject>& jsonObjectPtr, FOsmRelationMember& member)
{
	return false;
}

bool AEarth::LoadNodeFromJsonObject(const TSharedPtr<FJsonObject>&jsonObjectPtr)
{
	FOsmNode node;
	
	if (!jsonObjectPtr->TryGetNumberField("id", node.id))
	{
		UE_LOG(LogTemp, Error, TEXT("Node ID field not found in JSON!"));
		return false;
	}
	if (!jsonObjectPtr->TryGetNumberField("lat", node.lat))
	{
		UE_LOG(LogTemp, Error, TEXT("Node Lat field not found in JSON!"));
		return false;
	}
	if (!jsonObjectPtr->TryGetNumberField("lon", node.lon))
	{
		UE_LOG(LogTemp, Error, TEXT("Node Lon field not found in JSON!"));
		return false;
	}

	LoadTagsFromJsonArray(node, jsonObjectPtr);

	osmNodes.Add(node.id, node);

	return true;
}

bool AEarth::LoadWayFromJsonObject(const TSharedPtr<FJsonObject>& jsonObjectPtr)
{
	FOsmWay way;

	if (!jsonObjectPtr->TryGetNumberField("id", way.id))
	{
		UE_LOG(LogTemp, Error, TEXT("Way ID field not found in JSON!"));
		return false;
	}
	if (jsonObjectPtr->HasField("nodes"))
	{
		auto nodesArray = jsonObjectPtr->GetArrayField("nodes");
		for (auto& nodeIdJson : nodesArray)
		{
			int64 nodeId;
			if (!nodeIdJson->TryGetNumber(nodeId))
			{
				UE_LOG(LogTemp, Error, TEXT("Way node id is not a number!"));
				return false;
			}
			way.nodeIds.Add(nodeId);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Way with no nodes ignored!"));
	}	

	LoadTagsFromJsonArray(way, jsonObjectPtr);

	osmWays.Add(way.id, way);

	return true;
}

bool AEarth::LoadRelationFromJsonObject(const TSharedPtr<FJsonObject>& jsonObjectPtr)
{
	FOsmRelation relation;

	if (!jsonObjectPtr->TryGetNumberField("id", relation.id))
	{
		UE_LOG(LogTemp, Error, TEXT("Relation ID field not found in JSON!"));
		return false;
	}
	if (jsonObjectPtr->HasField("members"))
	{
		auto membersArray = jsonObjectPtr->GetArrayField("members");
		for (auto& memberJson : membersArray)
		{
			TSharedPtr<FJsonObject>* memberJsonObject;
			if (!memberJson->TryGetObject(memberJsonObject))
			{
				UE_LOG(LogTemp, Error, TEXT("Relation members field is not a JSON object!"));
				return false;
			}
			FOsmRelationMember member;
			if (!LoadRelationMemberFromJsonObject(*memberJsonObject, member))
			{
				return false;
			}
			relation.members.Add(member);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Relation with no members ignored!"));
	}

	LoadTagsFromJsonArray(relation, jsonObjectPtr);

	osmRelations.Add(relation.id, relation);

	return true;
}

bool AEarth::LoadFromJsonObject(const FJsonObjectWrapper& jsonObjectWrapper)
{
	int nodeNum = 0;
	int wayNum = 0;
	int relationNum = 0;

	FJsonObject jsonObject = *jsonObjectWrapper.JsonObject.Get();
	auto elementsArray = jsonObject.GetArrayField("elements");

	for (auto& value : elementsArray)
	{
		auto osmElementJsonObj = value->AsObject();
		if (!osmElementJsonObj->HasField("type"))
		{
			continue;
		}

		FString type = osmElementJsonObj->GetStringField("type");
		if (type == "node")
		{
			if (!LoadNodeFromJsonObject(osmElementJsonObj))
			{
				return false;
			}
			nodeNum++;
		}
		else if (type == "way")
		{
			if (!LoadWayFromJsonObject(osmElementJsonObj))
			{
				return false;
			}
			wayNum++;
		}
		else if (type == "rel")
		{
			if (!LoadRelationFromJsonObject(osmElementJsonObj))
			{
				return false;
			}
			relationNum++;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Unknown OSM element type %s!"), *type);
		}		
	}

	UE_LOG(LogTemp, Display, TEXT("Loaded OSM elements: %d nodes, %d ways, %d relations."), nodeNum, wayNum, relationNum);

	return true;
}

const TMap<int64, FOsmNode>& AEarth::GetNodes()
{
	return osmNodes;
}

const TMap<int64, FOsmWay>& AEarth::GetWays()
{
	return osmWays;
}

const TMap<int64, FOsmRelation>& AEarth::GetRelations()
{
	return osmRelations;
}

void AEarth::BuildSpatialIndex()
{
	FVector2D coordinateCenter(0, 0);
	FVector2D halfAngleSize(90, 180);
	FLatLonBoundingBox globalBox(coordinateCenter, halfAngleSize);

	nodeSpatialIndex.Reset(new FQuadTree<int64>(globalBox));

	for (const auto& nodePair : osmNodes)
	{
		int64 nodeId = nodePair.Key;
		const FOsmNode& node = nodePair.Value;
		nodeSpatialIndex->Insert(node.GetLatLon(), nodeId);
	}

	DebugDrawSpatialIndex(5.0f);
}

void AEarth::DebugDrawGeoLine(const FVector2D& latLonFrom, const FVector2D& latLonTo, double angleStep, const FColor& color, float time) const
{
	double dLat = latLonTo.X - latLonFrom.X;
	double dLon = latLonTo.Y - latLonFrom.Y;
	double stepLat;
	double stepLon;
	int steps;
	if (FMath::IsNearlyZero(dLat) && FMath::IsNearlyZero(dLon))
	{
		return;
	}
	else if (FMath::Abs(dLat) > FMath::Abs(dLon))
	{
		stepLat = FMath::Sign(dLat) * angleStep;
		stepLon = FMath::Abs(dLon / dLat) * FMath::Sign(dLon) * angleStep;
		steps = FMath::Abs(dLat / stepLat);
	}
	else
	{
		stepLon = FMath::Sign(dLon) * angleStep;
		stepLat = FMath::Abs(dLat / dLon) * FMath::Sign(dLat) * angleStep;
		steps = FMath::Abs(dLon / stepLon);
	}	 
	
	for (int i = 0; i < steps - 1; i++)
	{
		double lat0 = latLonFrom.X + i * stepLat;
		double lon0 = latLonFrom.Y + i * stepLon;
		double lat1 = latLonFrom.X + (i + 1) * stepLat;
		double lon1 = latLonFrom.Y + (i + 1) * stepLon;
		FVector start = LatLonToWorldSpace(FVector2D(lat0, lon0));
		FVector end = LatLonToWorldSpace(FVector2D(lat1, lon1));
		DrawDebugLine(GetWorld(), start, end, color, false, time);
	}
}

void AEarth::DebugDrawQuadTreeNode(FQuadTree<int64>* quadTreeNode, double time) const
{
	check(quadTreeNode);

	double angleStep = 0.1f;
	FLatLonBoundingBox box = quadTreeNode->GetLatLonBoundingBox();

	FVector2D nw = box.GetNorthWestPoint();
	FVector2D ne = box.GetNorthEastPoint();
	FVector2D se = box.GetSouthEastPoint();
	FVector2D sw = box.GetSouthWestPoint();

	DebugDrawGeoLine(ne, nw, angleStep, FColor::Green, time);
	DebugDrawGeoLine(nw, sw, angleStep, FColor::Green, time);
	DebugDrawGeoLine(sw, se, angleStep, FColor::Green, time);
	DebugDrawGeoLine(se, ne, angleStep, FColor::Green, time);

	for (auto tree : quadTreeNode->GetSubtrees())
	{
		DebugDrawQuadTreeNode(tree, time);
	}
}

void AEarth::DebugDrawSpatialIndex(double time) const
{
	DebugDrawQuadTreeNode(nodeSpatialIndex.Get(), time);
}

void AEarth::GetBuildingRenderParameters(const FOsmWay& building, FVector& location, FQuat& rotation, FVector& scale)
{
	int64 nodeId0 = building.nodeIds[0];
	const FOsmNode& node0 = osmNodes[nodeId0];
	double latSum = node0.lat;
	double lonSum = node0.lon;

	double 
		latMin = node0.lat,
		lonMin = node0.lon,
		latMax = node0.lat,
		lonMax = node0.lon;

	for (int i = 1; i < building.nodeIds.Num(); i++)
	{
		int64 nodeId = building.nodeIds[i];
		const FOsmNode& node = osmNodes[nodeId];
		latSum += node.lat;
		lonSum += node.lon;

		if (node.lat < latMin)
		{
			latMin = node.lat;
		}
		else if (node.lat > latMax)
		{
			latMax = node.lat;
		}

		if (node.lon < lonMin)
		{
			lonMin = node.lon;
		}
		else if (node.lon > lonMax)
		{
			lonMax = node.lon;
		}
	}

	double latCenter = latSum / building.nodeIds.Num();
	double lonCenter = lonSum / building.nodeIds.Num();

	location = LatLonToWorldSpace(FVector2D(latCenter, lonCenter));

	if (FMath::RandRange(0.0, 1.0) > 0.9)
	{
		DrawDebugLine(GetWorld(), GetActorLocation(), location, FColor::Blue, true);
	}
	
	FRotator rotator(latCenter, lonCenter, 0);
	rotation = FQuat::MakeFromRotator(rotator);

	FVector minPoint = LatLonToWorldSpace(FVector2D(latMin, lonMin));
	FVector maxPoint = LatLonToWorldSpace(FVector2D(latMax, lonMax));
	double height = FMath::Abs(maxPoint.Y - minPoint.Y);
	double width = FMath::Abs(maxPoint.X - minPoint.X);
	scale.X = width * 0.01f;
	scale.Y = height * 0.01f;

	// TODO Scale by building height
	scale.Z = (width + height) / 2.0 * 0.01f;
}


void AEarth::RenderBuildings()
{
	if (!buildingVisualizer)
	{
		return;
	}

	// TODO Draw only visible buildings

	TArray<FVector> locations;
	TArray<FVector> scales;
	TArray<FQuat> rotations;
	for (const auto& wayTuple : osmWays)
	{
		const FOsmWay& way = wayTuple.Value;
		if (!way.tags.Contains("building"))
		{
			continue;
		}
		FVector location;
		FQuat rotation;
		FVector scale;
		GetBuildingRenderParameters(way, location, rotation, scale);
		locations.Add(location);
		rotations.Add(rotation);
		scales.Add(scale);
		//scales.Add(FVector(0.1f, 0.1f, 0.1f));
	}
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(buildingVisualizer, "TransformLocations", locations); UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(buildingVisualizer, "TransformLocations", locations);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayQuat(buildingVisualizer, "TransformRotations", rotations);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(buildingVisualizer, "TransformScales", scales);
}




// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HAL/FileManagerGeneric.h"
#include "OsmUtilsLibrary.generated.h"

class AEarth;

/**
 * 
 */
UCLASS()
class OSMVISUALISATIONPLUGIN_API UOsmUtilsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	static TArray<FString> SplitFilePath(const FString& filePath, char separator);

	static FString ReconstructPath(const TArray<FString>& pathSegments, char pathSeparator);

	static bool DoesMatchPattern(const FString& str, const FString& pattern, char patternMatchingChar);

	UFUNCTION(BlueprintCallable)
	static TArray<FString> GetFilesMatchingPattern(const FString& pattern, const FString& patternMatchingCharStr);

	UFUNCTION(BlueprintCallable, Category = "OSM", meta = (WorldContext = "WorldContextObject"))

	static void BuildEarthFromJsonFilesPattern(const UObject* WorldContextObject, AEarth* earth,  const FString& jsonFilesPattern, const FString& patternMatcher = "*");
	
	UFUNCTION(BlueprintCallable, meta = (WorldContext = WorldContextObject))
	static void BuildEarthFromJsonFile(const UObject* WorldContextObject, AEarth* earth, const FString& jsonFilesPattern);

};

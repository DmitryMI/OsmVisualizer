// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EarthViewerPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class OSMVISUALIZER_API AEarthViewerPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	void BeginPlay() override;
};

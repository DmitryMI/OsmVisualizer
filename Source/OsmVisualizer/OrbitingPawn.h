// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "OrbitingPawn.generated.h"

UCLASS()
class OSMVISUALIZER_API AOrbitingPawn : public APawn
{
	GENERATED_BODY()

private:
	UPROPERTY()
	USpringArmComponent* springArm;

	UPROPERTY(EditDefaultsOnly)
	double mouseMoveSensitivity = 500;

	UPROPERTY(EditDefaultsOnly)
	double mouseZoomSensitivity = 1000.0;

public:
	// Sets default values for this pawn's properties
	AOrbitingPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void OnMouseX(float value);

	void OnMouseY(float value);

	void OnMouseWheel(float value);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};

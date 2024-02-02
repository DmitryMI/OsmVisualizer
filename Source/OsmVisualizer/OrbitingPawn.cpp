// Fill out your copyright notice in the Description page of Project Settings.


#include "OrbitingPawn.h"

// Sets default values
AOrbitingPawn::AOrbitingPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AOrbitingPawn::BeginPlay()
{
	Super::BeginPlay();
	
	springArm = GetComponentByClass<USpringArmComponent>();
	ensure(springArm);
}

void AOrbitingPawn::OnMouseX(float value)
{
	if (!springArm)
	{
		return;
	}

	FRotator rotation = springArm->GetComponentRotation();
	rotation.Yaw += value * mouseMoveSensitivity * GetWorld()->GetDeltaSeconds();

	springArm->SetWorldRotation(rotation);
}

void AOrbitingPawn::OnMouseY(float value)
{
	if (!springArm)
	{
		return;
	}

	FRotator rotation = springArm->GetComponentRotation();
	rotation.Pitch += value * mouseMoveSensitivity * GetWorld()->GetDeltaSeconds();

	rotation.Pitch = FMath::Clamp(rotation.Pitch, -60, 60);

	springArm->SetWorldRotation(rotation);
	
}

void AOrbitingPawn::OnMouseWheel(float value)
{
	if (!springArm)
	{
		return;
	}

	springArm->TargetArmLength -= value * GetWorld()->GetDeltaSeconds() * mouseZoomSensitivity;
}


// Called every frame
void AOrbitingPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AOrbitingPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MouseX", this, &AOrbitingPawn::OnMouseX);
	PlayerInputComponent->BindAxis("MouseY", this, &AOrbitingPawn::OnMouseY);
	PlayerInputComponent->BindAxis("MouseWheel", this, &AOrbitingPawn::OnMouseWheel);
}


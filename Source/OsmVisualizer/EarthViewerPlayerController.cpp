// Fill out your copyright notice in the Description page of Project Settings.


#include "EarthViewerPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "EarthViewer.h"
#include "OrbitingPawn.h"

void AEarthViewerPlayerController::BeginPlay()
{
	Super::BeginPlay();

	AActor* earthViewerActor = UGameplayStatics::GetActorOfClass(GetWorld(), AEarthViewer::StaticClass());

	AOrbitingPawn* pawn = GetPawn<AOrbitingPawn>();
	if (ensure(pawn) && ensure(earthViewerActor))
	{
		pawn->AttachToActor(earthViewerActor, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
}
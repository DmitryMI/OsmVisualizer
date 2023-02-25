// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LogListViewItem.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class OSMVISUALISATIONPLUGINEDITOR_API ULogListViewItem : public UObject
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere)
	FString logLineText;

public:
	UFUNCTION(BlueprintCallable)
	void SetLogLineText(const FString& line);

	UFUNCTION(BlueprintCallable)
	const FString& GetLogLineText();

	UFUNCTION(BlueprintCallable)
	FString GetLogLineTextCopy();
};

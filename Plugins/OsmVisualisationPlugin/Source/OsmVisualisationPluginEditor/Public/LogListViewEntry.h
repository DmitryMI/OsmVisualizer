// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Components/TextBlock.h"
#include "LogListViewEntry.generated.h"

/**
 *
 */
UCLASS()
class OSMVISUALISATIONPLUGINEDITOR_API ULogListViewEntry : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
private:

protected:
	// IUserObjectListEntry
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	// IUserObjectListEntry

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* LogLine;

public:

};

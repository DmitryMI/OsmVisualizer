// Fill out your copyright notice in the Description page of Project Settings.


#include "LogListViewEntry.h"
#include "LogListViewItem.h"

void ULogListViewEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	ULogListViewItem* item = Cast<ULogListViewItem>(ListItemObject);
	check(item);

	FText text = FText::FromString(item->GetLogLineText());
	LogLine->SetText(text);
}


// Fill out your copyright notice in the Description page of Project Settings.


#include "LogListViewItem.h"

void ULogListViewItem::SetLogLineText(const FString& line)
{
	logLineText = line;
}

const FString& ULogListViewItem::GetLogLineText()
{
	return logLineText;
}

FString ULogListViewItem::GetLogLineTextCopy()
{
	return logLineText;
}

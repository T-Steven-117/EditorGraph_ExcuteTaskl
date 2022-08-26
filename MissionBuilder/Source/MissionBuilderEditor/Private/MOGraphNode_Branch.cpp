// Copyright Epic Games, Inc. All Rights Reserved.

#include "MOGraphNode_Branch.h"
#include "MissionBuilderEditorTypes.h"
#include "TaskObject.h"


void UMOGraphNode_Branch::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, UMissionBuilderEditorTypes::PinCategory_MultipleNodes, TEXT("In"));
	CreatePin(EGPD_Output, UMissionBuilderEditorTypes::PinCategory_SingleTask, TEXT("False"));
	CreatePin(EGPD_Output, UMissionBuilderEditorTypes::PinCategory_SingleTask, TEXT("True"));
}

void UMOGraphNode_Branch::GetPinHoverText(const UEdGraphPin& Pin, FString& HoverTextOut) const
{
	if (Pin.PinName.ToString().Equals(TEXT("True")))
	{
		HoverTextOut = TEXT("真");
	}
	else if (Pin.PinName.ToString().Equals(TEXT("False")))
	{
		HoverTextOut = TEXT("假");
	}
	else
	{
		Super::GetPinHoverText(Pin, HoverTextOut);
	}
}


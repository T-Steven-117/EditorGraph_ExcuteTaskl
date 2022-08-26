// Copyright Epic Games, Inc. All Rights Reserved.

#include "MOGraphNode_Parallel.h"
#include "MissionBuilderEditorTypes.h"
#include "TaskObject.h"


void UMOGraphNode_Parallel::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, UMissionBuilderEditorTypes::PinCategory_MultipleNodes, TEXT("In"));
	CreatePin(EGPD_Output, UMissionBuilderEditorTypes::PinCategory_SingleTask, TEXT("Out"));
	CreatePin(EGPD_Output, UMissionBuilderEditorTypes::PinCategory_MultipleNodes, TEXT("Parallel"));
}

void UMOGraphNode_Parallel::GetPinHoverText(const UEdGraphPin& Pin, FString& HoverTextOut) const
{
	if (Pin.PinName.ToString().Equals(TEXT("Parallel")))
	{
		HoverTextOut = TEXT("并行流");
	}
	else
	{
		Super::GetPinHoverText(Pin, HoverTextOut);
	}
}


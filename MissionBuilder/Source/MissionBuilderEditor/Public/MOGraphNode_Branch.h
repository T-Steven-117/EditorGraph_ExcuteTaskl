// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "MOGraphNode.h"
#include "MOGraphNode_Task.h"
#include "MOGraphNode_Branch.generated.h"

UCLASS()
class UMOGraphNode_Branch : public UMOGraphNode_Task
{
	GENERATED_BODY()

public:
	virtual void AllocateDefaultPins() override;

	/**
	 * Fetch the hover text for a pin when the graph is being edited.
	 *
	 * @param   Pin				The pin to fetch hover text for (should belong to this node)
	 * @param   HoverTextOut	This will get filled out with the requested text
	 */
	virtual void GetPinHoverText(const UEdGraphPin& Pin, FString& HoverTextOut) const override;
};

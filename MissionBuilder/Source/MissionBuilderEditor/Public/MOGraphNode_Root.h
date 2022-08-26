// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "MOGraphNode.h"
#include "MOGraphNode_Root.generated.h"

/** Root node of this behavior tree, holds Blackboard data */
UCLASS()
class UMOGraphNode_Root : public UMOGraphNode
{
	GENERATED_UCLASS_BODY()

	//UPROPERTY(EditAnywhere, Category="AI|MO")
	//class UBlackboardData* BlackboardAsset;

	virtual void PostPlacedNewNode() override;
	virtual void AllocateDefaultPins() override;
	virtual bool CanDuplicateNode() const override { return false; }
	virtual bool CanUserDeleteNode() const override{ return false; }
	virtual bool HasErrors() const override { return false; }
	virtual bool RefreshNodeClass() override { return false; }
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	/** gets icon resource name for title bar */
	virtual FName GetNameIcon() const override;
	virtual FText GetTooltipText() const override;

	virtual void PostEditChangeProperty( struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditUndo() override;
	virtual FText GetDescription() const override;

	/** notify behavior tree about blackboard change */
	//void UpdateBlackboard();
};

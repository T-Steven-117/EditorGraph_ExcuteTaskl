// Copyright Epic Games, Inc. All Rights Reserved.

#include "MOGraphNode_Root.h"
#include "UObject/UObjectIterator.h"
#include "MissionBuilderEditorTypes.h"
#include "MOGraph.h"
//#include "MO/BlackboardData.h"
//#include "MO/MO.h"

UMOGraphNode_Root::UMOGraphNode_Root(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bIsReadOnly = true;
}

void UMOGraphNode_Root::PostPlacedNewNode()
{
	Super::PostPlacedNewNode();

	// pick first available blackboard asset, hopefully something will be loaded...
	//for (FObjectIterator It(UBlackboardData::StaticClass()); It; ++It)
	//{
	//	UBlackboardData* TestOb = (UBlackboardData*)*It;
	//	if (!TestOb->HasAnyFlags(RF_ClassDefaultObject))
	//	{
	//		BlackboardAsset = TestOb;
	//		UpdateBlackboard();
	//		break;
	//	}
	//}
}

void UMOGraphNode_Root::AllocateDefaultPins()
{
	CreatePin(EGPD_Output, UMissionBuilderEditorTypes::PinCategory_SingleTask, TEXT("In"));
}

FText UMOGraphNode_Root::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return NSLOCTEXT("MOEditor", "Root", "ROOT");
}

FName UMOGraphNode_Root::GetNameIcon() const
{
	return FName("BTEditor.Graph.BTNode.Root.Icon");
}

FText UMOGraphNode_Root::GetTooltipText() const
{
	return UEdGraphNode::GetTooltipText();
}

void UMOGraphNode_Root::PostEditChangeProperty( struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	//if (PropertyChangedEvent.Property &&
	//	PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UMOGraphNode_Root, BlackboardAsset))
	//{
	//	UpdateBlackboard();
	//}
}

void UMOGraphNode_Root::PostEditUndo()
{
	Super::PostEditUndo();
	//UpdateBlackboard();
}

FText UMOGraphNode_Root::GetDescription() const
{
	return FText::FromString(TEXT("Root Node"));//GetNameSafe(BlackboardAsset));
}

//void UMOGraphNode_Root::UpdateBlackboard()
//{
//	UMOGraph* MyGraph = GetMOGraph();
//	UMO* BTAsset = Cast<UMO>(MyGraph->GetOuter());
//	if (BTAsset && BTAsset->BlackboardAsset != BlackboardAsset)
//	{
//		BTAsset->BlackboardAsset = BlackboardAsset;
//		MyGraph->UpdateBlackboardChange();
//	}
//}

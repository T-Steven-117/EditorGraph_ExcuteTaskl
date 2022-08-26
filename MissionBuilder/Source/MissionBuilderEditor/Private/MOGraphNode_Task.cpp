// Copyright Epic Games, Inc. All Rights Reserved.

#include "MOGraphNode_Task.h"
//#include "MO/BTNode.h"
#include "MissionBuilderEditorTypes.h"
#include "TaskObject.h"

UMOGraphNode_Task::UMOGraphNode_Task(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UMOGraphNode_Task::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, UMissionBuilderEditorTypes::PinCategory_MultipleNodes, TEXT("In"));
	CreatePin(EGPD_Output, UMissionBuilderEditorTypes::PinCategory_SingleTask, TEXT("Out"));
}

FText UMOGraphNode_Task::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	const UTaskObject* MyNode = Cast<UTaskObject>(NodeInstance);
	if (MyNode != NULL)
	{
		return MyNode->TaskName.IsEmpty() ? FText::FromString(MyNode->GetName()) : MyNode->TaskName;
	}
	else if (!ClassData.GetClassName().IsEmpty())
	{
		FString StoredClassName = ClassData.GetClassName();
		StoredClassName.RemoveFromEnd(TEXT("_C"));

		return FText::Format(NSLOCTEXT("MOGraph", "NodeClassError", "Class {0} not found, make sure it's saved!"), FText::FromString(StoredClassName));
	}

	return Super::GetNodeTitle(TitleType);
}

FText UMOGraphNode_Task::GetDescription() const
{
	// TODO: 节点中间的描述由对应NodeInstance实现
	if (!ClassData.GetClassName().IsEmpty())
	{
		FString StoredClassName = ClassData.GetClassName();
		StoredClassName.RemoveFromEnd(TEXT("_C"));

		return FText::FromString(StoredClassName);
	}

	return Super::GetDescription();
}

void UMOGraphNode_Task::OnUpdateCommentText(const FString& NewComment)
{
	UEdGraphNode::OnUpdateCommentText(NewComment);

	UTaskObject* MyNode = Cast<UTaskObject>(NodeInstance);
	if (MyNode != nullptr)
	{
		MyNode->Note = NewComment;
	}
}

void UMOGraphNode_Task::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	Super::GetNodeContextMenuActions(Menu, Context);
	//AddContextMenuActionsDecorators(Menu, "MOGraphNode", Context);
	//AddContextMenuActionsServices(Menu, "MOGraphNode", Context);
}

void UMOGraphNode_Task::UpdateNodeInfo()
{
	UTaskObject* MyNode = Cast<UTaskObject>(NodeInstance);
	if (MyNode != nullptr)
	{
		// 更新节点备注
		NodeComment = MyNode->Note;

		// 更新节点名称

		// 更新节点描述

	}
}

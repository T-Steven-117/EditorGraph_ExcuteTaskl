// Copyright Epic Games, Inc. All Rights Reserved.

#include "EdGraphSchema_MissionObject.h"
#include "Layout/SlateRect.h"
#include "EdGraphNode_Comment.h"
#include "Modules/ModuleManager.h"
#include "MissionBuilderEditorTypes.h"
#include "EdGraph/EdGraph.h"
#include "MOGraph.h"
#include "MOGraphNode.h"
#include "MOGraphNode_Root.h"
#include "MOGraphNode_Task.h"
#include "MOGraphNode_Branch.h"
#include "MOGraphNode_Parallel.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ToolMenus.h"
#include "ObjectEditorUtils.h"
#include "MissionBuilderEditor.h"
#include "IMissionObjectEditor.h"
//#include "MissionObjectDebugger.h"
#include "GraphEditorActions.h"
#include "MOGraphConnectionDrawingPolicy.h"
#include "Toolkits/ToolkitManager.h"
#include "MissionObject.h"
#include "UObject/UObjectIterator.h"
#include "TaskObject.h"
#include "Tasks/Task_Parallel.h"
#include "Tasks/Task_Branch.h"
//#include "MissionObject/Tasks/BTTask_RunBehavior.h"
//#include "MissionObject/Composites/BTComposite_SimpleParallel.h"
//#include "MissionObjectGraphNode_SimpleParallel.h"
//#include "MissionObjectGraphNode_SubtreeTask.h"

#define LOCTEXT_NAMESPACE "MissionObjectEditor"

int32 UEdGraphSchema_MissionObject::CurrentCacheRefreshID = 0;

//----------------------------------------------------------------------//
// 
//----------------------------------------------------------------------//
UEdGraphNode* FMissionObjectSchemaAction_AutoArrange::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
{
	UMOGraph* Graph = Cast<UMOGraph>(ParentGraph);
	if (Graph)
	{
		//Graph->AutoArrange();
	}

	return NULL;
}

//----------------------------------------------------------------------//
// 
//----------------------------------------------------------------------//

UEdGraphSchema_MissionObject::UEdGraphSchema_MissionObject(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UEdGraphSchema_MissionObject::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	FGraphNodeCreator<UMOGraphNode_Root> NodeCreator(Graph);
	UMOGraphNode_Root* MyNode = NodeCreator.CreateNode();
	NodeCreator.Finalize();
	SetNodeMetaData(MyNode, FNodeMetadata::DefaultGraphNode);
}

void UEdGraphSchema_MissionObject::GetGraphNodeContextActions(FGraphContextMenuBuilder& ContextMenuBuilder, int32 SubNodeFlags) const
{
	Super::GetGraphNodeContextActions(ContextMenuBuilder, SubNodeFlags);

	//if (SubNodeFlags == ESubNode::Decorator)
	//{
	//	const FText Category = FObjectEditorUtils::GetCategoryText(UMOGraphNode_CompositeDecorator::StaticClass());
	//	UEdGraph* Graph = (UEdGraph*)ContextMenuBuilder.CurrentGraph;
	//	UMOGraphNode_CompositeDecorator* OpNode = NewObject<UMOGraphNode_CompositeDecorator>(Graph);
	//	TSharedPtr<FAISchemaAction_NewSubNode> AddOpAction = UAIGraphSchema::AddNewSubNodeAction(ContextMenuBuilder, Category, FText::FromString(OpNode->GetNodeTypeDescription()), FText::GetEmpty());
	//	AddOpAction->ParentNode = Cast<UMOGraphNode>(ContextMenuBuilder.SelectedObjects[0]);
	//	AddOpAction->NodeTemplate = OpNode;
	//}
}

void UEdGraphSchema_MissionObject::GetSubNodeClasses(int32 SubNodeFlags, TArray<FGraphNodeClassData>& ClassData, UClass*& GraphNodeClass) const
{
	//FMissionBuilderEditorModule& EditorModule = FModuleManager::GetModuleChecked<FMissionBuilderEditorModule>(TEXT("MissionBuilderEditor"));
	//FGraphNodeClassHelper* ClassCache = EditorModule.GetClassCache().Get();

	//if (SubNodeFlags == ESubNode::Decorator)
	//{
	//	ClassCache->GatherClasses(UBTDecorator::StaticClass(), ClassData);
	//	GraphNodeClass = UMOGraphNode_Decorator::StaticClass();
	//}
	//else
	//{
	//	ClassCache->GatherClasses(UBTService::StaticClass(), ClassData);
	//	GraphNodeClass = UMOGraphNode_Service::StaticClass();
	//}

		//TArray<UClass*> Subclasses;
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (It->IsChildOf(UTaskObject::StaticClass()) && !It->HasAnyClassFlags(CLASS_Abstract))
		{
			//Subclasses.Add(*It);
			ClassData.Emplace(*It, TEXT(""));
		}
	}
}

void UEdGraphSchema_MissionObject::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	const FName PinCategory = ContextMenuBuilder.FromPin ?
		ContextMenuBuilder.FromPin->PinType.PinCategory : 
		UMissionBuilderEditorTypes::PinCategory_MultipleNodes;

	const bool bNoParent = (ContextMenuBuilder.FromPin == NULL);
	const bool bOnlyTasks = (PinCategory == UMissionBuilderEditorTypes::PinCategory_SingleTask);
	const bool bOnlyComposites = (PinCategory == UMissionBuilderEditorTypes::PinCategory_SingleComposite);
	const bool bAllowComposites = bNoParent || !bOnlyTasks || bOnlyComposites;
	const bool bAllowTasks = bNoParent || !bOnlyComposites || bOnlyTasks;

	FMissionBuilderEditorModule& EditorModule = FModuleManager::GetModuleChecked<FMissionBuilderEditorModule>(TEXT("MissionBuilderEditor"));
	FGraphNodeClassHelper* ClassCache = EditorModule.GetClassCache().Get();

	//if (bAllowComposites)
	//{
	//	FCategorizedGraphActionListBuilder CompositesBuilder(TEXT("Composites"));

	//	TArray<FGraphNodeClassData> NodeClasses;
	//	ClassCache->GatherClasses(UBTCompositeNode::StaticClass(), NodeClasses);

	//	const FString ParallelClassName = UBTComposite_SimpleParallel::StaticClass()->GetName();
	//	for (const auto& NodeClass : NodeClasses)
	//	{
	//		const FText NodeTypeName = FText::FromString(FName::NameToDisplayString(NodeClass.ToString(), false));

	//		TSharedPtr<FAISchemaAction_NewNode> AddOpAction = UAIGraphSchema::AddNewNodeAction(CompositesBuilder, NodeClass.GetCategory(), NodeTypeName, FText::GetEmpty());

	//		UClass* GraphNodeClass = UMOGraphNode_Composite::StaticClass();
	//		if (NodeClass.GetClassName() == ParallelClassName)
	//		{
	//			GraphNodeClass = UMOGraphNode_SimpleParallel::StaticClass();
	//		}

	//		UMOGraphNode* OpNode = NewObject<UMOGraphNode>(ContextMenuBuilder.OwnerOfTemporaries, GraphNodeClass);
	//		OpNode->ClassData = NodeClass;
	//		AddOpAction->NodeTemplate = OpNode;
	//	}

	//	ContextMenuBuilder.Append(CompositesBuilder);
	//}

	if (bAllowTasks)
	{
		FCategorizedGraphActionListBuilder TasksBuilder(TEXT("Tasks"));

		TArray<FGraphNodeClassData> NodeClasses;
		ClassCache->GatherClasses(UTaskObject::StaticClass(), NodeClasses);

		for (const auto& NodeClass : NodeClasses)
		{
			// 跳过基类
			if (NodeClass.GetClassName() == UTaskObject::StaticClass()->GetName())
				continue;

			const FText NodeTypeName = FText::FromString(FName::NameToDisplayString(NodeClass.ToString(), false));

			TSharedPtr<FMOSchemaAction_NewNode> AddOpAction = UMOGraphSchema::AddNewNodeAction(TasksBuilder, NodeClass.GetCategory(), NodeTypeName, FText::GetEmpty());
			
			// 一般Task节点
			UClass* GraphNodeClass = UMOGraphNode_Task::StaticClass();

			// Branch节点
			if (NodeClass.GetClassName() == UTask_Branch::StaticClass()->GetName())
			{
				GraphNodeClass = UMOGraphNode_Branch::StaticClass();
			}

			// Parallel节点
			if (NodeClass.GetClassName() == UTask_Parallel::StaticClass()->GetName())
			{
				GraphNodeClass = UMOGraphNode_Parallel::StaticClass();
			}

			UMOGraphNode* OpNode = NewObject<UMOGraphNode>(ContextMenuBuilder.OwnerOfTemporaries, GraphNodeClass);
			OpNode->ClassData = NodeClass;
			AddOpAction->NodeTemplate = OpNode;
		}

		ContextMenuBuilder.Append(TasksBuilder);
	}
	
	//if (bNoParent)
	//{
	//	TSharedPtr<FMissionObjectSchemaAction_AutoArrange> Action = TSharedPtr<FMissionObjectSchemaAction_AutoArrange>(
	//		new FMissionObjectSchemaAction_AutoArrange(FText::GetEmpty(), LOCTEXT("AutoArrange", "Auto Arrange"), FText::GetEmpty(), 0)
	//		);

	//	ContextMenuBuilder.AddAction(Action);
	//}
}

void UEdGraphSchema_MissionObject::GetContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	/*if (Context->Node && !Context->Pin)
	{
		const UMOGraphNode* BTGraphNode = Cast<const UMOGraphNode>(Context->Node);
		if (BTGraphNode && BTGraphNode->CanPlaceBreakpoints())
		{
			FToolMenuSection& Section = Menu->AddSection("EdGraphSchemaBreakpoints", LOCTEXT("BreakpointsHeader", "Breakpoints"));
			{
				Section.AddMenuEntry(FGraphEditorCommands::Get().ToggleBreakpoint);
				Section.AddMenuEntry(FGraphEditorCommands::Get().AddBreakpoint);
				Section.AddMenuEntry(FGraphEditorCommands::Get().RemoveBreakpoint);
				Section.AddMenuEntry(FGraphEditorCommands::Get().EnableBreakpoint);
				Section.AddMenuEntry(FGraphEditorCommands::Get().DisableBreakpoint);
			}
		}
	}*/

	Super::GetContextMenuActions(Menu, Context);
}

const FPinConnectionResponse UEdGraphSchema_MissionObject::CanCreateConnection(const UEdGraphPin* PinA, const UEdGraphPin* PinB) const
{
	// Make sure the pins are not on the same node
	if (PinA->GetOwningNode() == PinB->GetOwningNode())
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorSameNode","Both are on the same node"));
	}

	const bool bPinAIsSingleComposite = (PinA->PinType.PinCategory == UMissionBuilderEditorTypes::PinCategory_SingleComposite);
	const bool bPinAIsSingleTask = (PinA->PinType.PinCategory == UMissionBuilderEditorTypes::PinCategory_SingleTask);
	const bool bPinAIsSingleNode = (PinA->PinType.PinCategory == UMissionBuilderEditorTypes::PinCategory_SingleNode);

	const bool bPinBIsSingleComposite = (PinB->PinType.PinCategory == UMissionBuilderEditorTypes::PinCategory_SingleComposite);
	const bool bPinBIsSingleTask = (PinB->PinType.PinCategory == UMissionBuilderEditorTypes::PinCategory_SingleTask);
	const bool bPinBIsSingleNode = (PinB->PinType.PinCategory == UMissionBuilderEditorTypes::PinCategory_SingleNode);

	const bool bPinAIsTask = PinA->GetOwningNode()->IsA(UMOGraphNode_Task::StaticClass());
	//const bool bPinAIsComposite = PinA->GetOwningNode()->IsA(UMOGraphNode_Composite::StaticClass());
	
	const bool bPinBIsTask = PinB->GetOwningNode()->IsA(UMOGraphNode_Task::StaticClass());
	//const bool bPinBIsComposite = PinB->GetOwningNode()->IsA(UMOGraphNode_Composite::StaticClass());

	/*if ((bPinAIsSingleComposite && !bPinBIsComposite) || (bPinBIsSingleComposite && !bPinAIsComposite))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorOnlyComposite","Only composite nodes are allowed"));
	}

	if ((bPinAIsSingleTask && !bPinBIsTask) || (bPinBIsSingleTask && !bPinAIsTask))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorOnlyTask","Only task nodes are allowed"));
	}*/

	// Compare the directions
	if ((PinA->Direction == EGPD_Input) && (PinB->Direction == EGPD_Input))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorInput", "Can't connect input node to input node"));
	}
	else if ((PinB->Direction == EGPD_Output) && (PinA->Direction == EGPD_Output))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorOutput", "Can't connect output node to output node"));
	}

	class FNodeVisitorCycleChecker
	{
	public:
		/** Check whether a loop in the graph would be caused by linking the passed-in nodes */
		bool CheckForLoop(UEdGraphNode* StartNode, UEdGraphNode* EndNode)
		{
			VisitedNodes.Add(EndNode);
			return TraverseInputNodesToRoot(StartNode);
		}

	private:
		/** 
		 * Helper function for CheckForLoop()
		 * @param	Node	The node to start traversal at
		 * @return true if we reached a root node (i.e. a node with no input pins), false if we encounter a node we have already seen
		 */
		bool TraverseInputNodesToRoot(UEdGraphNode* Node)
		{
			VisitedNodes.Add(Node);

			// Follow every input pin until we cant any more ('root') or we reach a node we have seen (cycle)
			for (int32 PinIndex = 0; PinIndex < Node->Pins.Num(); ++PinIndex)
			{
				UEdGraphPin* MyPin = Node->Pins[PinIndex];

				if (MyPin->Direction == EGPD_Input)
				{
					for (int32 LinkedPinIndex = 0; LinkedPinIndex < MyPin->LinkedTo.Num(); ++LinkedPinIndex)
					{
						UEdGraphPin* OtherPin = MyPin->LinkedTo[LinkedPinIndex];
						if( OtherPin )
						{
							UEdGraphNode* OtherNode = OtherPin->GetOwningNode();
							if (VisitedNodes.Contains(OtherNode))
							{
								return false;
							}
							else
							{
								return TraverseInputNodesToRoot(OtherNode);
							}
						}
					}
				}
			}

			return true;
		}

		TSet<UEdGraphNode*> VisitedNodes;
	};
	// TODO: 修复有Branch时检查循环会出错
	// check for cycles
	FNodeVisitorCycleChecker CycleChecker;
	if(!CycleChecker.CheckForLoop(PinA->GetOwningNode(), PinB->GetOwningNode()))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorcycle", "Can't create a graph cycle"));
	}

	const bool bPinASingleLink = /*bPinAIsSingleComposite || */bPinAIsSingleTask || bPinAIsSingleNode;
	const bool bPinBSingleLink = /*bPinBIsSingleComposite || */bPinBIsSingleTask || bPinBIsSingleNode;

	if (PinB->Direction == EGPD_Input && PinB->LinkedTo.Num() > 0)
	{
		// 输入是接受多个连接
		if (!bPinBSingleLink)
		{
			// 直接连接，断掉A
			return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_A, LOCTEXT("PinConnectReplace", "Replace connection"));
		}

		if(bPinASingleLink)
		{
			return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_AB, LOCTEXT("PinConnectReplace", "Replace connection"));
		}
		else
		{
			//return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_B, LOCTEXT("PinConnectReplace", "Replace connection"));
			return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, LOCTEXT("PinConnect", "Connect nodes"));
		}
	}
	else if (PinA->Direction == EGPD_Input && PinA->LinkedTo.Num() > 0)
	{
		// 输入是接受多个连接
		if (!bPinASingleLink)
		{
			// 直接连接，断掉B
			return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_B, LOCTEXT("PinConnectReplace", "Replace connection"));
		}

		if (bPinBSingleLink)
		{
			return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_AB, LOCTEXT("PinConnectReplace", "Replace connection"));
		}
		else
		{
			//return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_A, LOCTEXT("PinConnectReplace", "Replace connection"));
			return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, LOCTEXT("PinConnect", "Connect nodes"));
		}
	}

	if (bPinASingleLink && PinA->LinkedTo.Num() > 0)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_A, LOCTEXT("PinConnectReplace", "Replace connection"));
	}
	else if(bPinBSingleLink && PinB->LinkedTo.Num() > 0)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_B, LOCTEXT("PinConnectReplace", "Replace connection"));
	}

	return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, LOCTEXT("PinConnect", "Connect nodes"));
}

const FPinConnectionResponse UEdGraphSchema_MissionObject::CanMergeNodes(const UEdGraphNode* NodeA, const UEdGraphNode* NodeB) const
{
	// Make sure the nodes are not the same 
	if (NodeA == NodeB)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Both are the same node"));
	}

	//const bool bNodeAIsDecorator = NodeA->IsA(UMOGraphNode_Decorator::StaticClass()) || NodeA->IsA(UMOGraphNode_CompositeDecorator::StaticClass());
	//const bool bNodeAIsService = NodeA->IsA(UMOGraphNode_Service::StaticClass());
	//const bool bNodeBIsComposite = NodeB->IsA(UMOGraphNode_Composite::StaticClass());
	//const bool bNodeBIsTask = NodeB->IsA(UMOGraphNode_Task::StaticClass());
	//const bool bNodeBIsDecorator = NodeB->IsA(UMOGraphNode_Decorator::StaticClass()) || NodeB->IsA(UMOGraphNode_CompositeDecorator::StaticClass());
	//const bool bNodeBIsService = NodeB->IsA(UMOGraphNode_Service::StaticClass());

	//if (FMissionObjectDebugger::IsPIENotSimulating())
	//{
	//	if (bNodeAIsDecorator)
	//	{
	//		const UMOGraphNode* BTNodeA = Cast<const UMOGraphNode>(NodeA);
	//		const UMOGraphNode* BTNodeB = Cast<const UMOGraphNode>(NodeB);
	//		
	//		if (BTNodeA && BTNodeA->bInjectedNode)
	//		{
	//			return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("MergeInjectedNodeNoMove", "Can't move injected nodes!"));
	//		}

	//		if (BTNodeB && BTNodeB->bInjectedNode)
	//		{
	//			const UMOGraphNode* ParentNodeB = Cast<const UMOGraphNode>(BTNodeB->ParentNode);

	//			int32 FirstInjectedIdx = INDEX_NONE;
	//			for (int32 Idx = 0; Idx < ParentNodeB->Decorators.Num(); Idx++)
	//			{
	//				if (ParentNodeB->Decorators[Idx]->bInjectedNode)
	//				{
	//					FirstInjectedIdx = Idx;
	//					break;
	//				}
	//			}

	//			int32 NodeIdx = ParentNodeB->Decorators.IndexOfByKey(BTNodeB);
	//			if (NodeIdx != FirstInjectedIdx)
	//			{
	//				return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("MergeInjectedNodeAtEnd", "Decorators must be placed above injected nodes!"));
	//			}
	//		}

	//		if (BTNodeB && BTNodeB->Decorators.Num())
	//		{
	//			for (int32 Idx = 0; Idx < BTNodeB->Decorators.Num(); Idx++)
	//			{
	//				if (BTNodeB->Decorators[Idx]->bInjectedNode)
	//				{
	//					return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("MergeInjectedNodeAtEnd", "Decorators must be placed above injected nodes!"));
	//				}
	//			}
	//		}
	//	}

	//	if ((bNodeAIsDecorator && (bNodeBIsComposite || bNodeBIsTask || bNodeBIsDecorator))
	//		|| (bNodeAIsService && (bNodeBIsComposite || bNodeBIsTask || bNodeBIsService)))
	//	{
	//		return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));
	//	}
	//}

	return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT(""));
}

FLinearColor UEdGraphSchema_MissionObject::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
	return FColor::White;
}

class FConnectionDrawingPolicy* UEdGraphSchema_MissionObject::CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, class FSlateWindowElementList& InDrawElements, class UEdGraph* InGraphObj) const
{
	return new FMOGraphConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements, InGraphObj);
}

bool UEdGraphSchema_MissionObject::IsCacheVisualizationOutOfDate(int32 InVisualizationCacheID) const
{
	return CurrentCacheRefreshID != InVisualizationCacheID;
}

int32 UEdGraphSchema_MissionObject::GetCurrentVisualizationCacheID() const
{
	return CurrentCacheRefreshID;
}

void UEdGraphSchema_MissionObject::ForceVisualizationCacheClear() const
{
	++CurrentCacheRefreshID;
}

#undef LOCTEXT_NAMESPACE

// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionObjectEditor.h"
#include "MOGraph.h"
#include "MissionObjectGraph.h"
//#include "MOGraphSchema.h"
#include "EdGraphSchema_MissionObject.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "MOGraphNode.h"
#include "MOGraphNode_Root.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/Commands/GenericCommands.h"
#include "GraphEditorActions.h"
#include "EdGraphUtilities.h"
#include "HAL/PlatformApplicationMisc.h"
#include "EdGraphNode_Comment.h"

#include "MissionObject.h"
#include "Tasks/Task_SubMission.h"

#define LOCTEXT_NAMESPACE "MisisonObjectEditor"

const FName FMissionObjectEditor::ToolkitFName(TEXT("MisisonObjectEditor"));
const FName FMissionObjectEditor::PropertiesTabId(TEXT("MisisonObjectEditor_Properties"));
const FName FMissionObjectEditor::MainGraphTabId(TEXT("MisisonObjectEditor_MainGraph"));
const FName FMissionObjectEditor::AppIdentifier(TEXT("MisisonObjectEditorAppIdentifier"));

FMissionObjectEditor::FMissionObjectEditor()
{
	// 注册Undo
	UEditorEngine* Editor = (UEditorEngine*)GEngine;
	if (Editor != NULL)
	{
		Editor->RegisterForUndo(this);
	}
}

UMissionObject* FMissionObjectEditor::GetMissionObject() const
{
	return MissionObject;
}

void FMissionObjectEditor::SetMissionObject(UMissionObject* InMissioObject)
{
	MissionObject = InMissioObject;
}

void FMissionObjectEditor::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	// Add a new workspace menu category to the tab manager
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_MisisonObjectEditor", "Mission Object Editor"));
	//WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("NewWorkspaceMenu_MisisonObjectEditor", "New Mission Object Editor"));

	// We register the tab manager to the asset editor toolkit so we can use it in this editor
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	// Register the properties tab spawner within our tab manager
	// We provide the function with the identiefier for this tab and a shared pointer to the
	// SpawnPropertiesTab function within this editor class
	// Additionaly, we provide a name to be displayed, a category and the tab icon
	InTabManager->RegisterTabSpawner(PropertiesTabId, FOnSpawnTab::CreateSP(this, &FMissionObjectEditor::SpawnPropertiesTab))
		.SetDisplayName(LOCTEXT("PropertiesTab", "Details"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(MainGraphTabId, FOnSpawnTab::CreateSP(this, &FMissionObjectEditor::SpawnTab_MainGraph))
		.SetDisplayName(LOCTEXT("GraphTabName", "Mission Object"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef());
		//.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

}

void FMissionObjectEditor::UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	// Unregister the tab manager from the asset editor toolkit
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	// Unregister our custom tab from the tab manager, making sure it is cleaned up when the editor gets destroyed
	InTabManager->UnregisterTabSpawner(MainGraphTabId);
	//InTabManager->UnregisterTabSpawner(PropertiesTabId);
}

void FMissionObjectEditor::InitCustomAssetEditorEditor(const EToolkitMode::Type Mode, const TSharedPtr<class IToolkitHost>& InitToolkitHost, UMissionObject* InMissionObject)
{
	// Cache some values that will be used for our details view arguments
	const bool bIsUpdatable = false;
	const bool bAllowFavorites = true;
	const bool bIsLockable = false;

	// Set this InCustomAsset as our editing asset
	SetMissionObject(InMissionObject);

	// Create the layout of our custom asset editor
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_CustomAssetEditor_Layout_v1")
		->AddArea
		(
			// Create a vertical area and spawn the toolbar
			FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.1f)
				->SetHideTabWell(true)
				->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewSplitter()->SetOrientation(EOrientation::Orient_Horizontal)
				->Split
				(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.7f)
						->AddTab(MainGraphTabId, ETabState::OpenedTab)
				)
				->Split
				(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.3f)
						->AddTab(PropertiesTabId, ETabState::OpenedTab)
				)
			)
		);

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;

	CreateInternalWidgets();

	// Initialize our custom asset editor
	FAssetEditorToolkit::InitAssetEditor(
		Mode,
		InitToolkitHost,
		AppIdentifier,
		StandaloneDefaultLayout,
		bCreateDefaultStandaloneMenu,
		bCreateDefaultToolbar,
		(UObject*)MissionObject);

	// 默认显示MissionObject属性
	if (DetailsView.IsValid())
	{
		DetailsView->SetObject(MissionObject);
	}
}

FMissionObjectEditor::~FMissionObjectEditor()
{
	UEditorEngine* Editor = (UEditorEngine*)GEngine;
	if (Editor)
	{
		Editor->UnregisterForUndo(this);
	}

	// On destruction we reset our tab and details view 
	DetailsView.Reset();
	PropertiesTab.Reset();
}

FName FMissionObjectEditor::GetToolkitFName() const
{
	return ToolkitFName;
}

FText FMissionObjectEditor::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "MisisonObjectEditor");
}

FText FMissionObjectEditor::GetToolkitName() const
{
	return LOCTEXT("MisisonObjectEditorName", "MisisonObjectEditor");
}

FText FMissionObjectEditor::GetToolkitToolTipText() const
{
	return LOCTEXT("ToolTip", "MisisonObjectEditor");
}

FString FMissionObjectEditor::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "AnimationDatabase ").ToString();
}

FLinearColor FMissionObjectEditor::GetWorldCentricTabColorScale() const
{
	return FColor::Red;
}

void FMissionObjectEditor::PostUndo(bool bSuccess)
{
	if (bSuccess)
	{
		// Clear selection, to avoid holding refs to nodes that go away
		if (GraphEditorPtr.IsValid())
		{
			GraphEditorPtr->ClearSelectionSet();
			GraphEditorPtr->NotifyGraphChanged();
		}
		FSlateApplication::Get().DismissAllMenus();
	}
}

void FMissionObjectEditor::PostRedo(bool bSuccess)
{
	if (bSuccess)
	{
		// Clear selection, to avoid holding refs to nodes that go away
		if (GraphEditorPtr.IsValid())
		{
			GraphEditorPtr->ClearSelectionSet();
			GraphEditorPtr->NotifyGraphChanged();
		}
		FSlateApplication::Get().DismissAllMenus();
	}
}

bool FMissionObjectEditor::IsPropertyEditable() const
{
	//if (FBehaviorTreeDebugger::IsPIESimulating() || bForceDisablePropertyEdit)
	//{
	//	return false;
	//}

	TSharedPtr<SGraphEditor> FocusedGraphEd = GraphEditorPtr;
	return FocusedGraphEd.IsValid() && FocusedGraphEd->GetCurrentGraph() && FocusedGraphEd->GetCurrentGraph()->bEditable;
}

void FMissionObjectEditor::OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent)
{
	// 更新Node的信息
	UMissionObjectGraph* MyGraph = Cast<UMissionObjectGraph>(MissionObject->MOGraph);
	if (MyGraph)
	{
		for (auto n : MyGraph->Nodes)
		{
			UMOGraphNode* MONode = Cast<UMOGraphNode>(n);
			if (MONode)
			{
				MONode->UpdateNodeInfo();
			}
		}

		// 显示版本修改 - 更新节点UI显示（如：Task名称改变，更新节点Title)
		MyGraph->GetSchema()->ForceVisualizationCacheClear();
	}

	FName PropertyName = PropertyChangedEvent.GetPropertyName();
	// 修改的属性为子任务的任务资源 - 因为子任务引用的MissionObject资源为当前MissionOjectEditor编辑的MissionObject时，该属性置空，避免循环引用
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UTask_SubMission, MissionObjectAsset))
	{
		FObjectProperty* ObjProperty = CastField<FObjectProperty>(PropertyChangedEvent.Property);
		if (ObjProperty)
		{
			for (int32 i = 0; i < PropertyChangedEvent.GetNumObjectsBeingEdited(); ++i)
			{
				void* ValuePtr = PropertyChangedEvent.Property->ContainerPtrToValuePtr<void>(const_cast<UObject*>(PropertyChangedEvent.GetObjectBeingEdited(i)));	// 此处需强制修改
				UObject* Obj = ObjProperty->GetObjectPropertyValue(ValuePtr);
				// 子任务引用的MissionObject和当前Graph的相同
				if (Obj == GetMissionObject())
				{
					// 单纯地设置为空
					ObjProperty->SetObjectPropertyValue(ValuePtr, nullptr);

					// 提示错误信息
					FGraphPanelSelectionSet SelectedNodes = GraphEditorPtr->GetSelectedNodes();
					for (auto Node : SelectedNodes)
					{
						UMOGraphNode* MONode = Cast<UMOGraphNode>(Node);
						if (!MONode)
							continue;

						MONode->ErrorMessage = TEXT("选中的MissionObjectAsset不能是当前编辑的MissionObject, MissionObjectAsset置空!");
					}
				}
				else
				{
					// 取消错误信息
					FGraphPanelSelectionSet SelectedNodes = GraphEditorPtr->GetSelectedNodes();
					for (auto Node : SelectedNodes)
					{
						UMOGraphNode* MONode = Cast<UMOGraphNode>(Node);
						if (!MONode)
							continue;

						MONode->ErrorMessage = TEXT("");
					}
				}
			}
		}
	}
}

FGraphPanelSelectionSet FMissionObjectEditor::GetSelectedNodes() const
{
	if (GraphEditorPtr.IsValid())
		return GraphEditorPtr->GetSelectedNodes();

	return FGraphPanelSelectionSet();
}

void FMissionObjectEditor::CreateCommandList()
{
	if (GraphEditorCommands.IsValid())
	{
		return;
	}

	GraphEditorCommands = MakeShareable(new FUICommandList);

	// Can't use CreateSP here because derived editor are already implementing TSharedFromThis<FAssetEditorToolkit>
	// however it should be safe, since commands are being used only within this editor
	// if it ever crashes, this function will have to go away and be reimplemented in each derived class

	GraphEditorCommands->MapAction(FGenericCommands::Get().SelectAll,
		FExecuteAction::CreateRaw(this, &FMissionObjectEditor::SelectAllNodes),
		FCanExecuteAction::CreateRaw(this, &FMissionObjectEditor::CanSelectAllNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Delete,
		FExecuteAction::CreateRaw(this, &FMissionObjectEditor::DeleteSelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FMissionObjectEditor::CanDeleteNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Copy,
		FExecuteAction::CreateRaw(this, &FMissionObjectEditor::CopySelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FMissionObjectEditor::CanCopyNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Cut,
		FExecuteAction::CreateRaw(this, &FMissionObjectEditor::CutSelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FMissionObjectEditor::CanCutNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Paste,
		FExecuteAction::CreateRaw(this, &FMissionObjectEditor::PasteNodes),
		FCanExecuteAction::CreateRaw(this, &FMissionObjectEditor::CanPasteNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Duplicate,
		FExecuteAction::CreateRaw(this, &FMissionObjectEditor::DuplicateNodes),
		FCanExecuteAction::CreateRaw(this, &FMissionObjectEditor::CanDuplicateNodes)
	);

	GraphEditorCommands->MapAction(
		FGraphEditorCommands::Get().CreateComment,
		FExecuteAction::CreateRaw(this, &FMissionObjectEditor::OnCreateComment),
		FCanExecuteAction::CreateRaw(this, &FMissionObjectEditor::CanCreateComment)
	);
}

void FMissionObjectEditor::SelectAllNodes()
{
	if (GraphEditorPtr.IsValid())
		GraphEditorPtr->SelectAllNodes();
}

bool FMissionObjectEditor::CanSelectAllNodes() const
{
	return true;
}

void FMissionObjectEditor::DeleteSelectedNodes()
{
	if (!GraphEditorPtr.IsValid())
		return;

	const FScopedTransaction Transaction(FGenericCommands::Get().Delete->GetDescription());
	GraphEditorPtr->GetCurrentGraph()->Modify();

	const FGraphPanelSelectionSet SelectedNodes = GraphEditorPtr->GetSelectedNodes();
	GraphEditorPtr->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*NodeIt))
		{
			if (Node->CanUserDeleteNode())
			{
				Node->Modify();
				Node->DestroyNode();
			}
		}
	}
}

bool FMissionObjectEditor::CanDeleteNodes() const
{
	// If any of the nodes can be deleted then we should allow deleting
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node && Node->CanUserDeleteNode())
		{
			return true;
		}
	}

	return false;
}

void FMissionObjectEditor::DeleteSelectedDuplicatableNodes()
{
	if (!GraphEditorPtr.IsValid())
		return;

	const FGraphPanelSelectionSet OldSelectedNodes = GraphEditorPtr->GetSelectedNodes();
	GraphEditorPtr->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(OldSelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node && Node->CanDuplicateNode())
		{
			GraphEditorPtr->SetNodeSelection(Node, true);
		}
	}

	// Delete the duplicatable nodes
	DeleteSelectedNodes();

	GraphEditorPtr->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(OldSelectedNodes); SelectedIter; ++SelectedIter)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter))
		{
			GraphEditorPtr->SetNodeSelection(Node, true);
		}
	}
}

void FMissionObjectEditor::CutSelectedNodes()
{
	CopySelectedNodes();
	DeleteSelectedDuplicatableNodes();
}

bool FMissionObjectEditor::CanCutNodes() const
{
	return CanCopyNodes() && CanDeleteNodes();
}

void FMissionObjectEditor::CopySelectedNodes()
{

	// Export the selected nodes and place the text on the clipboard
	FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	TArray<UMOGraphNode*> SubNodes;

	FString ExportedText;

	int32 CopySubNodeIndex = 0;
	for (FGraphPanelSelectionSet::TIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		UMOGraphNode* MONode = Cast<UMOGraphNode>(Node);
		if (Node == nullptr)
		{
			SelectedIter.RemoveCurrent();
			continue;
		}

		Node->PrepareForCopying();

		if (MONode)
		{
			MONode->CopySubNodeIndex = CopySubNodeIndex;

			// append all subnodes for selection
			for (int32 Idx = 0; Idx < MONode->SubNodes.Num(); Idx++)
			{
				MONode->SubNodes[Idx]->CopySubNodeIndex = CopySubNodeIndex;
				SubNodes.Add(MONode->SubNodes[Idx]);
			}

			CopySubNodeIndex++;
		}
	}

	for (int32 Idx = 0; Idx < SubNodes.Num(); Idx++)
	{
		SelectedNodes.Add(SubNodes[Idx]);
		SubNodes[Idx]->PrepareForCopying();
	}

	FEdGraphUtilities::ExportNodesToText(SelectedNodes, ExportedText);
	FPlatformApplicationMisc::ClipboardCopy(*ExportedText);

	for (FGraphPanelSelectionSet::TIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UMOGraphNode* Node = Cast<UMOGraphNode>(*SelectedIter);
		if (Node)
		{
			Node->PostCopyNode();
		}
	}
}

bool FMissionObjectEditor::CanCopyNodes() const
{
	// If any of the nodes can be duplicated then we should allow copying
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node && Node->CanDuplicateNode())
		{
			return true;
		}
	}

	return false;
}

void FMissionObjectEditor::PasteNodes()
{
	if (!GraphEditorPtr.IsValid())
		return;

	PasteNodesHere(GraphEditorPtr->GetPasteLocation());
}

void FMissionObjectEditor::PasteNodesHere(const FVector2D& Location)
{
	if (!GraphEditorPtr.IsValid())
		return;

	// Undo/Redo support
	const FScopedTransaction Transaction(FGenericCommands::Get().Paste->GetDescription());
	UEdGraph* EdGraph = GraphEditorPtr->GetCurrentGraph();
	UMOGraph* MOGraph = Cast<UMOGraph>(EdGraph);

	EdGraph->Modify();
	if (MOGraph)
	{
		MOGraph->LockUpdates();
	}

	UMOGraphNode* SelectedParent = NULL;
	bool bHasMultipleNodesSelected = false;

	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UMOGraphNode* Node = Cast<UMOGraphNode>(*SelectedIter);
		if (Node && Node->IsSubNode())
		{
			Node = Node->ParentNode;
		}

		if (Node)
		{
			if (SelectedParent == nullptr)
			{
				SelectedParent = Node;
			}
			else
			{
				bHasMultipleNodesSelected = true;
				break;
			}
		}
	}

	// Clear the selection set (newly pasted stuff will be selected)
	GraphEditorPtr->ClearSelectionSet();

	// Grab the text to paste from the clipboard.
	FString TextToImport;
	FPlatformApplicationMisc::ClipboardPaste(TextToImport);

	// Import the nodes
	TSet<UEdGraphNode*> PastedNodes;
	FEdGraphUtilities::ImportNodesFromText(EdGraph, TextToImport, /*out*/ PastedNodes);

	//Average position of nodes so we can move them while still maintaining relative distances to each other
	FVector2D AvgNodePosition(0.0f, 0.0f);

	// Number of nodes used to calculate AvgNodePosition
	int32 AvgCount = 0;

	for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
	{
		UEdGraphNode* EdNode = *It;
		UMOGraphNode* AINode = Cast<UMOGraphNode>(EdNode);
		if (EdNode && (AINode == nullptr || !AINode->IsSubNode()))
		{
			AvgNodePosition.X += EdNode->NodePosX;
			AvgNodePosition.Y += EdNode->NodePosY;
			++AvgCount;
		}
	}

	if (AvgCount > 0)
	{
		float InvNumNodes = 1.0f / float(AvgCount);
		AvgNodePosition.X *= InvNumNodes;
		AvgNodePosition.Y *= InvNumNodes;
	}

	bool bPastedParentNode = false;

	TMap<int32, UMOGraphNode*> ParentMap;
	for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
	{
		UEdGraphNode* PasteNode = *It;
		UMOGraphNode* PasteAINode = Cast<UMOGraphNode>(PasteNode);

		if (PasteNode && (PasteAINode == nullptr || !PasteAINode->IsSubNode()))
		{
			bPastedParentNode = true;

			// Select the newly pasted stuff
			GraphEditorPtr->SetNodeSelection(PasteNode, true);

			PasteNode->NodePosX = (PasteNode->NodePosX - AvgNodePosition.X) + Location.X;
			PasteNode->NodePosY = (PasteNode->NodePosY - AvgNodePosition.Y) + Location.Y;

			PasteNode->SnapToGrid(16);

			// Give new node a different Guid from the old one
			PasteNode->CreateNewGuid();

			if (PasteAINode)
			{
				PasteAINode->RemoveAllSubNodes();
				ParentMap.Add(PasteAINode->CopySubNodeIndex, PasteAINode);
			}
		}
	}

	for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
	{
		UMOGraphNode* PasteNode = Cast<UMOGraphNode>(*It);
		if (PasteNode && PasteNode->IsSubNode())
		{
			PasteNode->NodePosX = 0;
			PasteNode->NodePosY = 0;

			// remove subnode from graph, it will be referenced from parent node
			PasteNode->DestroyNode();

			PasteNode->ParentNode = ParentMap.FindRef(PasteNode->CopySubNodeIndex);
			if (PasteNode->ParentNode)
			{
				PasteNode->ParentNode->AddSubNode(PasteNode, EdGraph);
			}
			else if (!bHasMultipleNodesSelected && !bPastedParentNode && SelectedParent)
			{
				PasteNode->ParentNode = SelectedParent;
				SelectedParent->AddSubNode(PasteNode, EdGraph);
			}
		}
	}

	if (MOGraph)
	{
		MOGraph->UpdateClassData();
		MOGraph->OnNodesPasted(TextToImport);
		MOGraph->UnlockUpdates();
	}

	// Update UI
	GraphEditorPtr->NotifyGraphChanged();

	UObject* GraphOwner = EdGraph->GetOuter();
	if (GraphOwner)
	{
		GraphOwner->PostEditChange();
		GraphOwner->MarkPackageDirty();
	}
}

bool FMissionObjectEditor::CanPasteNodes() const
{
	if (!GraphEditorPtr.IsValid())
		return false;

	FString ClipboardContent;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);

	return FEdGraphUtilities::CanImportNodesFromText(GraphEditorPtr->GetCurrentGraph(), ClipboardContent);
}

void FMissionObjectEditor::DuplicateNodes()
{
	CopySelectedNodes();
	PasteNodes();
}

bool FMissionObjectEditor::CanDuplicateNodes() const
{
	return CanCopyNodes();
}

bool FMissionObjectEditor::CanCreateComment() const
{
	return true;// 哪里都可以按C打备注 GraphEditorPtr.IsValid() ? (GraphEditorPtr->GetNumberOfSelectedNodes() != 0) : false;
}

void FMissionObjectEditor::OnCreateComment()
{
	if (UEdGraph* EdGraph = GraphEditorPtr.IsValid() ? GraphEditorPtr->GetCurrentGraph() : nullptr)
	{
		TSharedPtr<FEdGraphSchemaAction> Action = EdGraph->GetSchema()->GetCreateCommentAction();
		if (Action.IsValid())
		{
			FVector2D OpLocation;
			if (GraphEditorPtr)
				OpLocation = GraphEditorPtr->GetPasteLocation();
			Action->PerformAction(EdGraph, nullptr, OpLocation);
		}
	}
}

void FMissionObjectEditor::SaveAsset_Execute()
{
	UMissionObjectGraph* MyGraph = Cast<UMissionObjectGraph>(MissionObject->MOGraph);
	if (MyGraph)
		MyGraph->OnSave();

	// 调用父类方法
	FAssetEditorToolkit::SaveAsset_Execute();
}

TSharedRef<SDockTab> FMissionObjectEditor::SpawnPropertiesTab(const FSpawnTabArgs& Args)
{
	// Make sure we have the correct tab id
	check(Args.GetTabId() == PropertiesTabId);

	// Return a new slate dockable tab that contains our details view
	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("GenericEditor.Tabs.Properties"))
		.Label(LOCTEXT("GenericDetailsTitle", "Details"))
		.TabColorScale(GetTabColorScale())
		[
			// Provide the details view as this tab its content
			DetailsView.ToSharedRef()
		];
}

TSharedRef<SDockTab> FMissionObjectEditor::SpawnTab_MainGraph(const FSpawnTabArgs& Args)
{
	// Make sure we have the correct tab id
	check(Args.GetTabId() == MainGraphTabId);

	// 尝试获取MissionObject对应的Graph实例
	UMissionObjectGraph* MyGraph = Cast<UMissionObjectGraph>(MissionObject->MOGraph);
	// 无效则创建新的Graph
	if (MyGraph)
	{
		// 通知加载
		MyGraph->OnLoaded();
	}
	else
	{
		//创建图表对象
		MissionObject->MOGraph = FBlueprintEditorUtils::CreateNewGraph(MissionObject, TEXT("Mission Object"), UMissionObjectGraph::StaticClass(), UEdGraphSchema_MissionObject::StaticClass());
		MyGraph = Cast<UMissionObjectGraph>(MissionObject->MOGraph);
		// 默认创建根节点
		MyGraph->GetSchema()->CreateDefaultNodesForGraph(*MyGraph);
		// 通知创建
		MyGraph->OnCreated();
	}
	// 通知初始化
	MyGraph->Initialize();

	// 创建CommandList
	if (!GraphEditorCommands.IsValid())
	{
		CreateCommandList();
	}

	SGraphEditor::FGraphEditorEvents InEvents;
	InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FMissionObjectEditor::OnSelectedNodesChanged);
	InEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(this, &FMissionObjectEditor::OnNodeDoubleClicked);	// 节点双击
	InEvents.OnTextCommitted = FOnNodeTextCommitted::CreateSP(this, &FMissionObjectEditor::OnNodeTitleCommitted);	// 节点名称更改

	//创建图表编辑器控件
	GraphEditorPtr = SNew(SGraphEditor)
		.AdditionalCommands(GraphEditorCommands)
		.GraphEvents(InEvents)
		.GraphToEdit(MyGraph);

	// Return a new slate dockable tab that contains our details view
	return SNew(SDockTab)
		//.Icon(FEditorStyle::GetBrush("GenericEditor.Tabs.Properties"))
		.Label(LOCTEXT("GenericGraphTitle", "Mission Object"))
		.TabColorScale(GetTabColorScale())
		[
			// 提供详细信息视图作为该选项卡的内容  Provide the details view as this tab its content
			//DetailsView.ToSharedRef()
			GraphEditorPtr.ToSharedRef()
		];
}

void FMissionObjectEditor::CreateInternalWidgets()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs(false, false, true, FDetailsViewArgs::HideNameArea, false);
	//DetailsViewArgs.NotifyHook = this;
	DetailsViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Hide;
	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsView->SetObject(NULL);
	DetailsView->SetIsPropertyEditingEnabledDelegate(FIsPropertyEditingEnabled::CreateSP(this, &FMissionObjectEditor::IsPropertyEditable));
	DetailsView->OnFinishedChangingProperties().AddSP(this, &FMissionObjectEditor::OnFinishedChangingProperties);
}

void FMissionObjectEditor::OnSelectedNodesChanged(const TSet<class UObject*>& NewSelection)
{
	TArray<UObject*> Selection;

	for (UObject* SelectionEntry : NewSelection)
	{
		// MissionBuilder节点类
		UMOGraphNode* GraphNode = Cast<UMOGraphNode>(SelectionEntry);
		if (GraphNode && GraphNode->NodeInstance)
		{
			Selection.Add(GraphNode->NodeInstance);
			continue;
		}

		// 备注节点
		UEdGraphNode_Comment* CommentNode = Cast<UEdGraphNode_Comment>(SelectionEntry);
		if (CommentNode)
		{
			Selection.Add(SelectionEntry);
			continue;
		}
	}

	UMissionObjectGraph* MyGraph = Cast<UMissionObjectGraph>(MissionObject->MOGraph);
	//FAbortDrawHelper Mode0, Mode1;
	//bShowDecoratorRangeLower = false;
	//bShowDecoratorRangeSelf = false;
	//bForceDisablePropertyEdit = SelectionInfo.bInjectedNode;
	//bSelectedNodeIsInjected = SelectionInfo.bInjectedNode;
	//bSelectedNodeIsRootLevel = SelectionInfo.bRootLevelNode;

	if (Selection.Num() == 1)
	{
		if (DetailsView.IsValid())
		{
			DetailsView->SetObjects(Selection);
		}
	}
	else if (DetailsView.IsValid())
	{
		if (Selection.Num() == 0)
		{
			//// if nothing is selected, display the root
			//UMOGraphNode* RootNode = nullptr;
			//for (const auto& Node : MyGraph->Nodes)
			//{
			//	RootNode = Cast<UMOGraphNode_Root>(Node);
			//	if (RootNode != nullptr)
			//	{
			//		break;
			//	}
			//}

			//DetailsView->SetObject(RootNode);
			// 不点击Task节点则显示MissionObject属性
			DetailsView->SetObject(MissionObject);
		}
		else
		{
			// 多个节点
			DetailsView->SetObjects(Selection);
		}
	}

	//MyGraph->UpdateAbortHighlight(Mode0, Mode1);
}

void FMissionObjectEditor::OnNodeDoubleClicked(UEdGraphNode* Node)
{
	UMOGraphNode* MyNode = Cast<UMOGraphNode>(Node);
	
	if (MyNode && MyNode->NodeInstance)
	{
		// 子任务节点 - 打开子任务资源
		if (UTask_SubMission* SubMission = Cast<UTask_SubMission>(MyNode->NodeInstance))
		{
			if (SubMission->MissionObjectAsset)
			{
				GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(SubMission->MissionObjectAsset);
			}
		}
		// 打开蓝图节点
		else if (MyNode->NodeInstance->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
		{
			UClass* NodeClass = MyNode->NodeInstance->GetClass();
			UPackage* Pkg = NodeClass->GetOuterUPackage();
			FString ClassName = NodeClass->GetName().LeftChop(2);
			UBlueprint* BlueprintOb = FindObject<UBlueprint>(Pkg, *ClassName);
			if (BlueprintOb)
			{
				GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(BlueprintOb);
			}
		}
	}
}

void FMissionObjectEditor::OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged)
{
	if (NodeBeingChanged)
	{
		static const FText TranscationTitle = FText::FromString(FString(TEXT("Rename Node")));
		const FScopedTransaction Transaction(TranscationTitle);
		NodeBeingChanged->Modify();
		NodeBeingChanged->OnRenameNode(NewText.ToString());
	}
}



#undef LOCTEXT_NAMESPACE
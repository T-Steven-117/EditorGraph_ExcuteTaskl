// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUndoClient.h"
#include "IMissionObjectEditor.h"
//#include "IMissionObjectEditor.generated.h"

class UMissionObject;

/**
 * Public interface to Custom Asset Editor
 */
class MISSIONBUILDEREDITOR_API FMissionObjectEditor : public IMissionObjectEditor, public FEditorUndoClient
{
public:

	FMissionObjectEditor();

	/** Retrieves the current custom asset. */
	virtual UMissionObject* GetMissionObject() const override;

	/** Set the current custom asset. */
	virtual void SetMissionObject(UMissionObject* InMissioObject) override;

	// This function creates tab spawners on editor initialization
	virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager) override;

	// This function unregisters tab spawners on editor initialization
	virtual void UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager) override;

	/**
	 * Edits the specified asset object
	 *
	 * @param	Mode					Asset editing mode for this editor (standalone or world-centric)
	 * @param	InitToolkitHost			When Mode is WorldCentric, this is the level editor instance to spawn this editor within
	 * @param	InMissionObject			The MissionObject to Edit
	 */
	void InitCustomAssetEditorEditor(const EToolkitMode::Type Mode, const TSharedPtr<class IToolkitHost>& InitToolkitHost, UMissionObject* InMissionObject);

	/** Destructor */
	virtual ~FMissionObjectEditor();

	/** Begin IToolkit interface */
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FText GetToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual bool IsPrimaryEditor() const override { return true; }
	/** End IToolkit interface */

	//~ Begin FEditorUndoClient Interface
	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;
	// End of FEditorUndoClient

	bool IsPropertyEditable() const;
	void OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent);

public:
	
	FGraphPanelSelectionSet GetSelectedNodes() const;

	void CreateCommandList();

	// Delegates for graph editor commands
	void SelectAllNodes();
	bool CanSelectAllNodes() const;
	void DeleteSelectedNodes();
	bool CanDeleteNodes() const;
	void DeleteSelectedDuplicatableNodes();
	void CutSelectedNodes();
	bool CanCutNodes() const;
	void CopySelectedNodes();
	bool CanCopyNodes() const;
	void PasteNodes();
	void PasteNodesHere(const FVector2D& Location);
	bool CanPasteNodes() const;
	void DuplicateNodes();
	bool CanDuplicateNodes() const;

	bool CanCreateComment() const;
	void OnCreateComment();

protected:

	/** 保存按钮点击调用 */
	virtual void SaveAsset_Execute() override;

private:
	/** Create the properties tab and its content */
	TSharedRef<SDockTab> SpawnPropertiesTab(const FSpawnTabArgs& Args);
	/** Create the Graph tab and its content */
	TSharedRef<SDockTab> SpawnTab_MainGraph(const FSpawnTabArgs& Args);

	/** 创建DetailsView等Widget */
	void CreateInternalWidgets();

	/** Called when the selection changes in the GraphEditor */
	virtual void OnSelectedNodesChanged(const TSet<class UObject*>& NewSelection);
	/** 节点双击回调 */
	void OnNodeDoubleClicked(UEdGraphNode* Node);
	/** 节点名称改变提交（比如：备注节点（按C)评论提交） */
	void OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged);

private:

	/** Dockable tab for properties */
	TSharedPtr<SDockableTab> PropertiesTab;
	/** Dockable tab for Graph */
	TSharedPtr<SDockableTab> GraphTab;

	/** Details view */
	TSharedPtr<class IDetailsView> DetailsView;

	/**	The tab ids for all the tabs used */
	static const FName PropertiesTabId;
	/** Graph tab Id */
	static const FName MainGraphTabId;

	static const FName ToolkitFName;

	static const FName AppIdentifier;

	/** The Custom Asset open within this editor */
	UMissionObject* MissionObject;

private:

	//class UMissionObjectGraph* GraphObj;
	TSharedPtr<SGraphEditor> GraphEditorPtr;

	/** The command list for this editor */
	TSharedPtr<FUICommandList> GraphEditorCommands;
};

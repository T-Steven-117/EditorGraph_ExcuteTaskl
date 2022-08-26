// Copyright Epic Games, Inc. All Rights Reserved.

#include "MissionBuilderEditor.h"
#include "MissionObjectEditor.h"
#include "..\Public\MissionBuilderEditor.h"
#include "AssetToolsModule.h"
#include "AIGraphTypes.h"
#include "EdGraphUtilities.h"

#include "AssetTypeActions_MissionObject.h"
#include "MOGraphNode.h"
#include "SGraphNodeMO.h"
#include "SGraphNode_MissionObject.h"

#include "TaskObject.h"
#include "MissionObject.h"


#define LOCTEXT_NAMESPACE "MissionBuilderEditorModule"

class FGraphPanelNodeFactory_MissionObject : public FGraphPanelNodeFactory
{
	virtual TSharedPtr<class SGraphNode> CreateNode(UEdGraphNode* Node) const override
	{
		if (UMOGraphNode* MONode = Cast<UMOGraphNode>(Node))
		{
			return SNew(SGraphNode_MissionObject, MONode);
		}

		//if (UBehaviorTreeDecoratorGraphNode_Decorator* InnerNode = Cast<UBehaviorTreeDecoratorGraphNode_Decorator>(Node))
		//{
		//	return SNew(SGraphNode_Decorator, InnerNode);
		//}

		return NULL;
	}
};

TSharedPtr<FGraphPanelNodeFactory> GraphPanelNodeFactory_MissionObject;

void FMissionBuilderEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	// Create new extensibility managers for our menu and toolbar
	MenuExtensibilityManager = MakeShareable(new FExtensibilityManager);
	ToolBarExtensibilityManager = MakeShareable(new FExtensibilityManager);

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	RegisterAssetTypeAction(AssetTools, MakeShareable(new FAssetTypeActions_MissionObject()));

	GraphPanelNodeFactory_MissionObject = MakeShareable(new FGraphPanelNodeFactory_MissionObject());
	FEdGraphUtilities::RegisterVisualNodeFactory(GraphPanelNodeFactory_MissionObject);
}

void FMissionBuilderEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	// Reset our existing extensibility managers
	MenuExtensibilityManager.Reset();
	ToolBarExtensibilityManager.Reset();
	ClassCache.Reset();

	if (GraphPanelNodeFactory_MissionObject.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualNodeFactory(GraphPanelNodeFactory_MissionObject);
		GraphPanelNodeFactory_MissionObject.Reset();
	}

	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		// Unregister our custom created assets from the AssetTools
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (int32 i = 0; i < CreatedAssetTypeActions.Num(); ++i)
		{
			AssetTools.UnregisterAssetTypeActions(CreatedAssetTypeActions[i].ToSharedRef());
		}
	}

	CreatedAssetTypeActions.Empty();
}

TSharedRef<IMissionObjectEditor> FMissionBuilderEditorModule::CreateCustomAssetEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UMissionObject* MissionObject)
{
	if (!ClassCache.IsValid())
	{
		ClassCache = MakeShareable(new FGraphNodeClassHelper(UTaskObject::StaticClass()));
		//FGraphNodeClassHelper::AddObservedBlueprintClasses(UBTTask_BlueprintBase::StaticClass());
		//FGraphNodeClassHelper::AddObservedBlueprintClasses(UBTDecorator_BlueprintBase::StaticClass());
		//FGraphNodeClassHelper::AddObservedBlueprintClasses(UBTService_BlueprintBase::StaticClass());
		ClassCache->UpdateAvailableBlueprintClasses();
	}

	// Initialize and spawn a new custom asset editor with the provided parameters
	TSharedRef<FMissionObjectEditor> NewCustomAssetEditor(new FMissionObjectEditor());
	NewCustomAssetEditor->InitCustomAssetEditorEditor(Mode, InitToolkitHost, MissionObject);
	return NewCustomAssetEditor;
}

void FMissionBuilderEditorModule::RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
{
	AssetTools.RegisterAssetTypeActions(Action);
	CreatedAssetTypeActions.Add(Action);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMissionBuilderEditorModule, MissionBuilderEditor)
// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetTypeActions_MissionObject.h"
#include "../../MissionBuilder/Public/MissionObject.h"
#include "MissionBuilderEditor.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions_MissionObject"

FText FAssetTypeActions_MissionObject::GetName() const
{
	return LOCTEXT("AssetTypeActions_MissionObject", "MissionObject");
}

FColor FAssetTypeActions_MissionObject::GetTypeColor() const
{
	return FColor::Magenta;
}

UClass* FAssetTypeActions_MissionObject::GetSupportedClass() const
{
	return UMissionObject::StaticClass();
}

FText FAssetTypeActions_MissionObject::GetAssetDescription(const FAssetData& AssetData) const
{
	FString Description = AssetData.GetTagValueRef<FString>(GET_MEMBER_NAME_CHECKED(UMissionObject, MissionDescription));
	if (!Description.IsEmpty())
	{
		Description.ReplaceInline(TEXT("\\n"), TEXT("\n"));
		return FText::FromString(MoveTemp(Description));
	}

	return FText::GetEmpty();
}

void FAssetTypeActions_MissionObject::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto MissionObj = Cast<UMissionObject>(*ObjIt);
		if (MissionObj != nullptr)
		{
			IMissionBuilderEditorModule* MissionBuilderEditorModule = &FModuleManager::LoadModuleChecked<IMissionBuilderEditorModule>("MissionBuilderEditor");
			MissionBuilderEditorModule->CreateCustomAssetEditor(Mode, EditWithinLevelEditor, MissionObj);
		}
	}
}

uint32 FAssetTypeActions_MissionObject::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

#undef LOCTEXT_NAMESPACE
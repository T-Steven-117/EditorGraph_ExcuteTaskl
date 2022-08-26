

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MissionObject.generated.h"

class UTaskObject;

/**
 * 
 */
UCLASS(BlueprintType/*Blueprintable, EditInlineNew, DefaultToInstanced*/)
class MISSIONBUILDER_API UMissionObject : public UObject
{
	GENERATED_BODY()
	
public:
	///** 任务ID */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	//	int32 MissionID;

	/** 任务名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission", meta=(DisplayName="任务名称"))
		FString MissionName;
	/** 任务描述，在ContentBrowser悬浮此资源时能显示任务描述 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission", meta = (DisplayName = "任务描述",MultiLine = true), DuplicateTransient)
		FString MissionDescription;

	///** 任务小图标 */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	//	class UTexture2D* MissionIcon;

	/** 所有执行步骤 */
	UPROPERTY(/*EditAnywhere, Instanced, */BlueprintReadWrite, Category = "Mission")
		TArray<UTaskObject*> Tasks;

	/** 采用树结构的根Task */
	UPROPERTY(BlueprintReadWrite, Category = "Mission")
		UTaskObject* RootTask;

	//~ Begin UObject Interface (WITH_EDITOR)
	virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
	//~ End UObject Interface

#if WITH_EDITOR
	/**
	 * Called when a property on this object has been modified externally
	 *
	 * @param PropertyThatChanged the property that was modified
	 */
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	/**
	 * This alternate version of PostEditChange is called when properties inside structs are modified.  The property that was actually modified
	 * is located at the tail of the list.  The head of the list of the UStructProperty member variable that contains the property that was modified.
	 */
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent);

#endif // WITH_EDITOR

#if WITH_EDITORONLY_DATA

	/** Graph for MissionObject */
	UPROPERTY()
	class UEdGraph* MOGraph;

	/** Info about the graphs we last edited */
	UPROPERTY()
	TArray<FEditedDocumentInfo> LastEditedDocuments;	// TODO;

#endif
};

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "MOGraph.h"
#include "MissionObjectGraph.generated.h"

class UEdGraphPin;
class UTaskObject;

UCLASS()
class MISSIONBUILDEREDITOR_API UMissionObjectGraph : public UMOGraph
{
	GENERATED_BODY()
public:

	virtual void OnCreated() override;
	virtual void OnLoaded() override;
	virtual void Initialize() override;

	virtual void UpdateAsset(int32 UpdateFlags = 0) override;

	virtual void OnSubNodeDropped() override;
	virtual void OnNodesPasted(const FString& ImportStr) override;

	/** 保存通知 */
	void OnSave();

protected:
	virtual void OnNodeInstanceRemoved(UObject* NodeInstance) override;

	void CreateTaskFromNode(class UMOGraphNode* ParentNode, class UTaskObject* ParentTask, class UMissionObject* MO);
	void CreateTaskFromParallelNode(UMOGraphNode* ParallelNode, class UTask_Parallel* ParallelTask, class UMissionObject* MO);

#if WITH_EDITOR
	virtual void PostEditUndo() override;
#endif

private:
	void UpdateTaskID(UTaskObject* RootTask);

	void RecursionUpdateTaskID(int32& TaskID, UTaskObject* LastTask);

	UTaskObject* UpdateBranchTaskID(int32& TaskID, class UTask_Branch* Branch);

};

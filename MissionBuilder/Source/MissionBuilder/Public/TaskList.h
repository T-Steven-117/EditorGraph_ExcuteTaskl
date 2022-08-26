

#pragma once

#include "CoreMinimal.h"
#include "MissionObject.h"
#include "UObject/NoExportTypes.h"
#include "TaskList.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class MISSIONBUILDER_API UTaskList : public UObject
{
	GENERATED_BODY()

public:
	/** 所有任务 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "TaskList")
		TArray<UMissionObject*> Missions;
	
};

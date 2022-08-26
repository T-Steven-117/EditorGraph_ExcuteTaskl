

#pragma once

#include "CoreMinimal.h"
#include "../TaskObject.h"
#include "Task_Branch.generated.h"

class UBranchCallbackBase;
/**
 * 
 */
UCLASS()
class MISSIONBUILDER_API UTask_Branch : public UTaskObject
{
	GENERATED_BODY()

public:

	UTask_Branch();

	virtual void ExecuteTask(UObject* MissionManagerObj) override;

	virtual UTaskObject* GetNextTask() override;

public:
	/** Branch条件回调Object */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (DisplayName = "条件判断类"))
		TSubclassOf<UBranchCallbackBase> CallbackClass;

	/** 分支成功后执行的Task, NOTE: 默认NextTask为分支失败执行的Task (左假右真）*/
	UPROPERTY(BlueprintReadOnly)
		UTaskObject* NextTask2;
	
protected:
	UPROPERTY(BlueprintReadOnly)
		UBranchCallbackBase* CallbackInstance;
};

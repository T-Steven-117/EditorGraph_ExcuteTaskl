


#include "Tasks/Task_Branch.h"
#include "../Public/BranchCallbackBase.h"

UTask_Branch::UTask_Branch()
{
	TaskName = FText::FromString(TEXT("条件分支"));
#if WITH_EDITORONLY_DATA
	NodeColor = FLinearColor(0.8f, 0.f, 0.06f);
#endif
}

void UTask_Branch::ExecuteTask(UObject* MissionManagerObj)
{
	Super::ExecuteTask(MissionManagerObj);

	FinishExecute(true);
}

UTaskObject* UTask_Branch::GetNextTask()
{
	if (!CallbackInstance)
	{
		if (!CallbackClass)
		{
			UE_LOG(LogTemp, Error, TEXT("CallbackClass is empty!"));
			return nullptr;
		}

		CallbackInstance = NewObject<UBranchCallbackBase>(this, CallbackClass);
	}

	// 成功返回右边Task
	if (CallbackInstance->Evaluate())
	{
		return NextTask2;
	}

	// 失败返回左边Task
	return NextTask;
}

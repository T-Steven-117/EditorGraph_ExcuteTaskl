#include "../../Public/Tasks/Task_Delay.h"

void UTask_Delay::ExecuteTask(UObject* MissionManagerObj)
{
	Super::ExecuteTask(MissionManagerObj);

	if (GetWorld())
	{
		// 绑定延迟结束回调
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UTask_Delay::OnDelayEnd, Delay, false);
	}
}

UTask_Delay::UTask_Delay()
{
	Delay = 1.f;
	TaskName = FText::FromString(TEXT("延迟"));
#if WITH_EDITORONLY_DATA
	NodeColor = FLinearColor(0.1f, 0.8f, 0.0f);
#endif
}

void UTask_Delay::AbortTask()
{
	Super::AbortTask();

	// 清除Timer
	if(TimerHandle.IsValid())
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}

void UTask_Delay::OnDelayEnd()
{
	// 清除Timer
	if (TimerHandle.IsValid())
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	FinishExecute(true);
}




#include "Tasks/Task_ConditionJumpTaskBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "MissionManager.h"

//UTask_ConditionJumpTaskBase::UTask_ConditionJumpTaskBase()
//{
//	TaskName = FText::FromString(TEXT("条件跳到子任务"));
//
//#if WITH_EDITORONLY_DATA
//	Note = TEXT("只能在同一个Mission中跳");
//#endif
//}
//
//bool UTask_ConditionJumpTaskBase::CanJumpTask_Implementation()
//{
//	return false;
//}
//
//void UTask_ConditionJumpTaskBase::OnExecuteTask_Implementation()
//{
//	if (CanJumpTask())
//	{
//		UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
//		if (GI)
//		{
//			UMissionManager* MissionManager = GI->GetSubsystem<UMissionManager>();
//			if (MissionManager)
//			{
//				MissionManager->JumpToTask(TaskID_ToJump);
//			}
//		}
//	}
//	else
//	{
//		FinishExecute(true);
//	}
//}

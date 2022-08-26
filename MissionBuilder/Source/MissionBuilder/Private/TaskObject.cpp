


#include "TaskObject.h"
#include "GameFramework/Pawn.h"
#include "../Public/MissionManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UTaskObject::UTaskObject()
{
	//AllowMode = 4;
	//bEnable = true;
	bPersistentTick = false;

#if WITH_EDITORONLY_DATA
	NodeColor = FLinearColor(0.24f, 0.055f, 0.715f);
#endif
}

void UTaskObject::ExecuteTask(UObject* MissionManagerObj)
{

	MissionManagerPtr = Cast<IMissionInterface>(MissionManagerObj);

	UE_LOG(LogMissionManager, Log, TEXT("%s 开始执行Task %d: %s"), MissionManagerPtr ? *MissionManagerPtr->GetMissionName() : TEXT(""), TaskID, TaskName.IsEmpty() ? *GetName() : *TaskName.ToString());

	// 缓存玩家变换
	if (MissionManagerPtr != nullptr && MissionManagerPtr->GetPlayerPawn())
	{
		PlayerTransformCache = MissionManagerPtr->GetPlayerPawn()->GetActorTransform();
	}

	if (MissionManagerPtr)
	{
		if (bPersistentTick)
		{
			MissionManagerPtr->RegisterPersistentTick(this);
		}
	}

	OnExecuteTask();
}

void UTaskObject::AbortTask()
{
	UE_LOG(LogMissionManager, Log, TEXT("%s 中断Task %d: %s"), MissionManagerPtr ? *MissionManagerPtr->GetMissionName() : TEXT(""), TaskID, TaskName.IsEmpty() ? *GetName() : *TaskName.ToString());

	OnAbortTask();
}

void UTaskObject::ResetTask()
{
	UE_LOG(LogMissionManager, Warning, TEXT("%s 重置Task %d: %s"), MissionManagerPtr ? *MissionManagerPtr->GetMissionName() : TEXT(""), TaskID, TaskName.IsEmpty() ? *GetName() : *TaskName.ToString());
	// 恢复玩家变换
	if (MissionManagerPtr && MissionManagerPtr->GetPlayerPawn())
	{
		MissionManagerPtr->GetPlayerPawn()->SetActorTransform(PlayerTransformCache);
	}

	OnResetTask();
}

void UTaskObject::ClearTask()
{
	OnClearTask();
}

void UTaskObject::SkipTask(UObject* MissionManagerObj)
{
	MissionManagerPtr = Cast<IMissionInterface>(MissionManagerObj);

	UE_LOG(LogMissionManager, Log, TEXT("%s 跳过执行Task %d: %s"), MissionManagerPtr ? *MissionManagerPtr->GetMissionName() : TEXT(""), TaskID, TaskName.IsEmpty() ? *GetName() : *TaskName.ToString());

	// 缓存玩家变换
	if (MissionManagerPtr->GetPlayerPawn())
	{
		PlayerTransformCache = MissionManagerPtr->GetPlayerPawn()->GetActorTransform();
	}

	OnSkipTask();
}

bool UTaskObject::IsEnable() const
{
	return OnIsEnable();
}

void UTaskObject::FinishExecute(bool bSuccess/* = true*/)
{
	if (MissionManagerPtr)
	{
		MissionManagerPtr->OnTaskFinished(this, bSuccess);
	}

	// 回调通知
	OnTaskComplete.Broadcast(bSuccess);
	UE_LOG(LogMissionManager, Warning, TEXT("%s 完成Task %d: %s"), MissionManagerPtr ? *MissionManagerPtr->GetMissionName() : TEXT(""), TaskID, TaskName.IsEmpty() ? *GetName() : *TaskName.ToString());
}

void UTaskObject::Tick(float DeltaTime)
{
	OnTick(DeltaTime);
}

UTaskObject* UTaskObject::GetNextTask()
{
	return NextTask;
}

UWorld* UTaskObject::GetWorld() const
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
#if WITH_EDITOR
		if (GetOuter()->GetWorld() == nullptr)
		{
			// 编辑器（非PIE）下用于DrawDebugLine等函数的返回编辑器下的World
			for (TObjectIterator<UWorld> It; It; ++It)
			{
				if (It->WorldType == EWorldType::Editor)
				{
					return *It;
				}
			}
		}
#endif // WITH_EDITOR

		return GetOuter()->GetWorld();
	}

	return nullptr;
}

bool UTaskObject::OnIsEnable_Implementation() const
{
	return true;
}

void UTaskObject::StartPersistentTick()
{
	if (!MissionManagerPtr)
		return;

	MissionManagerPtr->RegisterPersistentTick(this);
}

void UTaskObject::StopPersistentTick()
{
	if (!MissionManagerPtr)
		return;

	MissionManagerPtr->UnRegisterPersistentTick(this);
}

AActor* UTaskObject::FindActorByTag(FName ActorTag)
{
	// TODO: 采用缓存来提高性能

	// 直接做查找
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClassWithTag(this, AActor::StaticClass(), ActorTag, FoundActors);
	if (FoundActors.Num() > 0)
	{
		if (FoundActors.Num() == 1)
		{
			return FoundActors[0];
		}
		else
		{
			TASKLOG(Error, FColor::Red, "%s : FindActorByTag()存在多个Tag为：%s 将采用第一个Actor", *GetFName().ToString(), *ActorTag.ToString());
			//UE_LOG(LogMissionManager, Error, TEXT("%s : FindActorByTag()存在多个Tag为：%s 将采用第一个Actor"), *GetFName().ToString(), *ActorTag.ToString());

			// 多个的话返回第一个
			return FoundActors[0];
		}
	}
	else
	{
		TASKLOG(Error, FColor::Red, "%s : FindActorByTag()无法找到Tag为：%s 的Actor", * GetFName().ToString(), *ActorTag.ToString());
		//UE_LOG(LogMissionManager, Error, TEXT("%s : FindActorByTag()无法找到Tag为：%s 的Actor"), *GetFName().ToString(), *ActorTag.ToString());
	}

	return nullptr;
}

TArray<AActor*> UTaskObject::FindActorsByTag(FName ActorTag)
{
	// TODO: 采用缓存来提高性能

	// 直接做查找
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClassWithTag(this, AActor::StaticClass(), ActorTag, FoundActors);

	if (FoundActors.Num() == 0)
	{
		TASKLOG(Warning, FColor::Yellow, "%s : FindActorsByTag无法找到Tag为：%s 的Actor", *GetFName().ToString(), *ActorTag.ToString());
	}

	return FoundActors;
}

UActorComponent* UTaskObject::FindComponentByTag(AActor* InActor, FName ComponentTag)
{
	if (!InActor)
	{
		TASKLOG(Error, FColor::Red, "%s : FindComponentByTag无法查找组件，给定Actor为空！", *GetFName().ToString());
		//UE_LOG(LogMissionManager, Error, TEXT("%s : FindComponentByTag无法查找组件，给定Actor为空！"), *GetFName().ToString());
		return nullptr;
	}

	TArray<UActorComponent*> Comps = InActor->GetComponentsByTag(UActorComponent::StaticClass(), ComponentTag);
	if (Comps.Num() > 0)
	{
		if (Comps.Num() > 1)
		{
			TASKLOG(Error, FColor::Red, "%s : FindComponentByTag存在多个Tag为：%s 的组件，将采用第一个", * GetFName().ToString(), * ComponentTag.ToString());
			//UE_LOG(LogMissionManager, Error, TEXT("%s : FindComponentByTag存在多个Tag为：%s 的组件，将采用第一个"), *GetFName().ToString(), *ComponentTag.ToString());
		}

		return Comps[0];
	}
	else
	{
		TASKLOG(Error, FColor::Red, "%s : FindComponentByTag组件Tag为：%s 的组件无法找到", *GetFName().ToString(), *ComponentTag.ToString());
		//UE_LOG(LogMissionManager, Error, TEXT("%s : FindComponentByTag组件Tag为：%s 的组件无法找到"), *GetFName().ToString(), *ComponentTag.ToString());
	}

	return nullptr;
}

TArray<UActorComponent*> UTaskObject::FindComponentsByTag(AActor* InActor, FName ComponentTag)
{
	if (!InActor)
	{
		TASKLOG(Error, FColor::Red, "%s : FindComponentsByTag无法查找组件，给定Actor为空！", *GetFName().ToString());
		//UE_LOG(LogMissionManager, Error, TEXT("%s : FindComponentByTag无法查找组件，给定Actor为空！"), *GetFName().ToString());
		return TArray<UActorComponent*>();
	}

	TArray<UActorComponent*> Comps = InActor->GetComponentsByTag(UActorComponent::StaticClass(), ComponentTag);
	if (Comps.Num() == 0)
	{
		TASKLOG(Error, FColor::Red, "%s : FindComponentsByTag组件Tag为：%s 的组件无法找到", *GetFName().ToString(), *ComponentTag.ToString());
	}

	return Comps;
}

USceneComponent* UTaskObject::FindSceneComponentByTag(AActor* InActor, FName ComponentTag)
{
	// 查找对应组件
	UActorComponent* FoundAcomp = FindComponentByTag(InActor, ComponentTag);
	if (USceneComponent* SceneComp = Cast<USceneComponent>(FoundAcomp))
	{
		return SceneComp;
	}
	else
	{
		TASKLOG(Error, FColor::Red, "%s : FindSceneComponentByTag组件Tag为：%s 的组件不是SceneComponent", *GetFName().ToString(), *ComponentTag.ToString());
		//UE_LOG(LogMissionManager, Error, TEXT("%s : FindSceneComponentByTag组件Tag为：%s 的组件不是SceneComponent"), *GetFName().ToString(), *ComponentTag.ToString());
	}

	return nullptr;
}

void UTaskObject::OnExecuteTask_Implementation()
{
}

void UTaskObject::OnAbortTask_Implementation()
{
}

void UTaskObject::OnResetTask_Implementation()
{
}

void UTaskObject::OnClearTask_Implementation()
{

}

void UTaskObject::OnSkipTask_Implementation()
{
}

void UTaskObject::OnTick_Implementation(float DeltaTime)
{
}

#include "../../Public/Tasks/Task_SpawnActorAttach.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UTask_SpawnActorAttach::UTask_SpawnActorAttach()
{
	TaskName = FText::FromString(TEXT("生成Actor附着"));
#if WITH_EDITORONLY_DATA
	NodeColor = FLinearColor(0.f, 0.f, 0.85f);
#endif
}

void UTask_SpawnActorAttach::ExecuteTask(UObject* MissionManagerObj)
{
	Super::ExecuteTask(MissionManagerObj);

	SpawnActor();
	FinishExecute(true);
}

void UTask_SpawnActorAttach::ResetTask()
{
	Super::ResetTask();

	// 存在则销毁
	if (SpawnedActor)
	{
		SpawnedActor->Destroy();
		SpawnedActor = nullptr;
	}
}

void UTask_SpawnActorAttach::SkipTask(UObject* MissionManagerObj)
{
	Super::SkipTask(MissionManagerObj);

	SpawnActor();
}

void UTask_SpawnActorAttach::ClearTask()
{
	Super::ClearTask();

	if (SpawnedActor)
	{
		SpawnedActor->Destroy();
	}

}
void UTask_SpawnActorAttach::SpawnActor()
{
	if (ActorClass && GetWorld())
	{
		FActorSpawnParameters ASP;
		ASP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnedActor = GetWorld()->SpawnActor<AActor>(ActorClass, ASP);

		if (SpawnedActor)
		{
			// 添加Actor Tag
			SpawnedActor->Tags.Add(ActorTag);

			AActor* ActorToAttach = FindActorByTag(ActorTagToAttach);
			if (ActorToAttach)
			{
				// 计算相对变换
				FTransform NewTransform = RelativeTransform;
				NewTransform.SetScale3D(RelativeTransform.GetScale3D() * SpawnedActor->GetActorRelativeScale3D());

				// 没有配置组件Tag，Attach到Actor根组件
				if (ComponentTagToAttach.IsNone())
				{
					FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
					AttachmentRules.LocationRule = EAttachmentRule::SnapToTarget;
					AttachmentRules.RotationRule = EAttachmentRule::SnapToTarget;
					AttachmentRules.ScaleRule = EAttachmentRule::KeepWorld;
					SpawnedActor->AttachToActor(ActorToAttach, AttachmentRules, SocketToAttach);

					// 设置相对变换
					SpawnedActor->SetActorRelativeTransform(NewTransform);
				}
				else
				{
					// 查找对应组件
					USceneComponent* SceneComp = FindSceneComponentByTag(ActorToAttach, ComponentTagToAttach);
					if (SceneComp)
					{
						FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
						AttachmentRules.LocationRule = EAttachmentRule::SnapToTarget;
						AttachmentRules.RotationRule = EAttachmentRule::SnapToTarget;
						AttachmentRules.ScaleRule = EAttachmentRule::KeepWorld;
						SpawnedActor->AttachToComponent(SceneComp, AttachmentRules, SocketToAttach);

						// 设置相对变换
						SpawnedActor->SetActorRelativeTransform(NewTransform);
					}
				}
			}
		}
		else
		{
			UE_LOG(LogMissionManager, Error, TEXT("生成Actor失败!"));
		}
	}
	else
	{
		UE_LOG(LogMissionManager, Error, TEXT("无法生成Actor，Actor类为空!"));
	}
}

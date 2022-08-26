#include "../../Public/Tasks/Task_SpawnActor.h"
#include "Engine/World.h"

UTask_SpawnActor::UTask_SpawnActor()
{
	TaskName = FText::FromString(TEXT("生成Actor"));
#if WITH_EDITORONLY_DATA
	NodeColor = FLinearColor(0.f, 0.1f, 0.6f);
#endif
}

void UTask_SpawnActor::ExecuteTask(UObject* MissionManagerObj)
{
	Super::ExecuteTask(MissionManagerObj);

	SpawnActor();
	FinishExecute(true);
}

void UTask_SpawnActor::ResetTask()
{
	Super::ResetTask();

	// 存在则销毁
	if (SpawnedActor)
	{
		SpawnedActor->Destroy();
		SpawnedActor = nullptr;
	}
}

void UTask_SpawnActor::ClearTask()
{
	Super::ClearTask();

	if (SpawnedActor)
	{
		SpawnedActor->Destroy();
	}

}

void UTask_SpawnActor::SkipTask(UObject* MissionManagerObj)
{
	Super::SkipTask(MissionManagerObj);

	SpawnActor();
}

void UTask_SpawnActor::SpawnActor()
{
	// Actor将要生成的变换
	FTransform FinalTransform;

	// 是否使用了相对变换
	if (bRelativeTransform)
	{
		// 寻找相对变换的Actor
		auto RelativeActor = FindActorByTag(RelativeActorTag);
		if (RelativeActor)
		{
			FTransform RelativeObjWorlTransfrom;
			// 配置的相对组件Tag是否为空
			if (!RelativeComponentTag.IsNone())
			{
				auto RelativeComp = FindSceneComponentByTag(RelativeActor, RelativeComponentTag);
				if (RelativeComp)
				{
					RelativeObjWorlTransfrom = RelativeComp->GetComponentTransform();
				}
			}
			else
			{
				// 使用Actor变换计算
				RelativeObjWorlTransfrom = RelativeActor->GetActorTransform();
			}

			// 计算配置的变换到世界变换
			FinalTransform.SetLocation(RelativeObjWorlTransfrom.TransformPosition(RelativeTransform.GetLocation()));
			FinalTransform.SetRotation(RelativeObjWorlTransfrom.TransformRotation(RelativeTransform.GetRotation()));
			FinalTransform.SetScale3D(RelativeObjWorlTransfrom.GetScale3D() * RelativeTransform.GetScale3D());
		}
	}
	else
	{
		FinalTransform = ActorTransform;
	}

	if (ActorClass && GetWorld())
	{
		FActorSpawnParameters ASP;
		ASP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnedActor = GetWorld()->SpawnActor(ActorClass, &FinalTransform, ASP);

		if (SpawnedActor)
		{
			// 添加Actor Tag
			SpawnedActor->Tags.Add(ActorTag);
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

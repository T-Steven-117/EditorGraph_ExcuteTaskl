#include "../../Public/Tasks/Task_ToggleVisibility.h"
#include "../../Public/TaskVisibilityInterface.h"


UTask_ToggleVisibility::UTask_ToggleVisibility()
{
	TaskName = FText::FromString(TEXT("显隐Actor或组件"));

#if WITH_EDITORONLY_DATA
	NodeColor = FLinearColor(0.2f, 0.5f, 0.7f);
#endif
}

void UTask_ToggleVisibility::ExecuteTask(UObject* MissionManagerObj)
{
	Super::ExecuteTask(MissionManagerObj);

	ToggleVisibility(Visible);
	FinishExecute(true);
}

void UTask_ToggleVisibility::ResetTask()
{
	Super::ResetTask();

	ToggleVisibility(!Visible);
}

void UTask_ToggleVisibility::SkipTask(UObject* MissionManagerObj)
{
	Super::SkipTask(MissionManagerObj);

	ToggleVisibility(Visible);
}

void UTask_ToggleVisibility::ToggleVisibility(bool IsVisible)
{
	TArray<AActor*> FoundActors = FindActorsByTag(ActorTag);
	if (FoundActors.Num() == 0)
		return;

	if (ComponentTag.IsNone())
	{
		for (auto a : FoundActors)
		{
			// 是否实现了TaskVisibility接口
			if (a->GetClass()->ImplementsInterface(UTaskVisibilityInterface::StaticClass()))
			{
				ITaskVisibilityInterface::Execute_ToggleVisibility(a, IsVisible);
			}
			else
			{
				a->SetActorHiddenInGame(!IsVisible);
			}

			// 处理碰撞
			if (IncludeCollision)
			{
				a->SetActorEnableCollision(IsVisible);
			}
		}
	}
	else // 配置了组件
	{
		for (auto a : FoundActors)
		{
			// 寻找组件
			TArray<UActorComponent*> FoundComps = FindComponentsByTag(a, ComponentTag);
			for (auto c : FoundComps)
			{
				// Primitive组件
				UPrimitiveComponent* Primi = Cast<UPrimitiveComponent>(c);

				// 是否实现了TaskVisibility接口
				if (c->GetClass()->ImplementsInterface(UTaskVisibilityInterface::StaticClass()))
				{
					ITaskVisibilityInterface::Execute_ToggleVisibility(c, IsVisible);
				}
				else
				{
					if (Primi)
						Primi->SetVisibility(IsVisible);
					else
					{
						TASKLOG(Error, FColor::Red, "%s :无法切换可见性，组件不是Primitive组件", *GetFName().ToString());
					}
				}

				// 处理碰撞
				if (IncludeCollision && Primi)
				{
					Primi->SetCollisionEnabled(IsVisible ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
				}
			}
		}
	}

}

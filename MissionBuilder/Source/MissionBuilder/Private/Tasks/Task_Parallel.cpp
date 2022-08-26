


#include "Tasks/Task_Parallel.h"
#include "../Public/Tasks/Task_SubMission.h"
#include "../Public/BranchCallbackBase.h"

// TODO:检查SubMission的IsEnable()是否开启执行

UTask_Parallel::UTask_Parallel()
{
	TaskName = FText::FromString(TEXT("并行执行"));
	bPersistentTick = true;

#if WITH_EDITORONLY_DATA
	NodeColor = FLinearColor(0.6f, 0.2f, 0.1f);
#endif
}

void UTask_Parallel::ExecuteTask(UObject* MissionManagerObj)
{
	Super::ExecuteTask(MissionManagerObj);

	for (auto RootTask : RootTasks)
	{
		// 没有Task跳过
		if (RootTask == nullptr)
			continue;

		// 创建子任务Object
		UTask_SubMission* NewSubM = NewObject<UTask_SubMission>(this);
		NewSubM->bPersistentTick = true;	// 子任务开始执行时会注册PersistentTick到当前Parallel

		// 创建任务执行Object
		UMissionObject* NewMO = NewObject<UMissionObject>(this);
		//NewMO->Tasks = TaskList.Tasks;
		NewMO->RootTask = RootTask;
		NewSubM->SetMissionObject(NewMO);

		// 添加到子任务列表
		SubMissions.Add(NewSubM);
		// 开始执行子任务
		NewSubM->ExecuteTask(this);
	}

	// 不存在子任务返回
	if (SubMissions.Num() == 0)
	{
		FinishExecute(true);
	}
}

void UTask_Parallel::AbortTask()
{
	Super::AbortTask();

	for (auto sm : SubMissions)
	{
		sm->AbortTask();
	}
}

void UTask_Parallel::ResetTask()
{
	Super::ResetTask();

	for (auto sm : SubMissions)
	{
		sm->ResetTask();
	}

}

void UTask_Parallel::SkipTask(UObject* MissionManagerObj)
{
	Super::SkipTask(MissionManagerObj);

	for (auto sm : SubMissions)
	{
		sm->SkipTask(this);
	}
}

void UTask_Parallel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 当前任务是否已经执行过Tick了
	bool IsCurrentTaskTicked = false;

	// 开始遍历
	bPersistentTickTasksLooping = true;
	// Tick持续性Tick的Task(默认的SubMission都是PersistentTick)
	for (auto TickTask : PersistentTickTasks)
	{
		TickTask->Tick(DeltaTime);

		//if (TickTask == CurrentTask)
		//	IsCurrentTaskTicked = true;
	}
	bPersistentTickTasksLooping = false;	// 结束遍历
// 存在需要移除的PersistentTickTasks - 遍历结束作移除
	if (PersistentTickTaskRemoveIndices.Num() > 0)
	{
		// 数组索引从大到小排序
		PersistentTickTaskRemoveIndices.Sort([](int32 L, int32 R) {
			return L > R;
		});

		for (auto index : PersistentTickTaskRemoveIndices)
		{
			PersistentTickTasks.RemoveAt(index, 1, false);
		}
		PersistentTickTaskRemoveIndices.Empty();

		// SubMission没有可以Tick的了，把自己从上一个MissionManager中移除持续Tick
		if (PersistentTickTasks.Num() == 0 && MissionManagerPtr)
		{
			MissionManagerPtr->UnRegisterPersistentTick(this);
		}
	}
	// 存在将要添加的Task到PersistentTickTasks
	if (PersistentTickTasksToAdd.Num() > 0)
	{
		for (auto i : PersistentTickTasksToAdd)
		{
			PersistentTickTasks.AddUnique(i);
		}
		PersistentTickTasksToAdd.Empty();

		// 尝试把自己从上一个MissionManager中添加持续Tick（避免当前不是Tick状态，突然有一个子任务要激活Tick）TODO: 优化此注册（判断自己非Tick时才注册自己）
		if (MissionManagerPtr)
		{
			MissionManagerPtr->RegisterPersistentTick(this);
		}
	}
}

void UTask_Parallel::OnTaskFinished(UTaskObject* Task, bool bSuccess)
{
	// 失败重新执行
	if (!bSuccess)
	{
		// 是否需要失败后再次执行
		if (Task->bReRunOnFalied)
		{
			Task->ExecuteTask(this);
		}
		else
		{
			TASKLOG(Error, FColor::Red, "中断执行！！");
		}
		// NOTE: 未配置bReRunOnFalied此处中断执行

		return;
	}

	// 添加到完成队列
	FinishedSubMissions.AddUnique(Task);
	// 从Tick中移除
	UnRegisterPersistentTick(Task);

	// 所有子任务完成，完成执行
	if (FinishedSubMissions.Num() == SubMissions.Num())
	{
		FinishExecute(true);
	}
}

void UTask_Parallel::RegisterPersistentTick(UTaskObject* Task)
{
	if (Task == nullptr)
		return;

	// 当前数组正在遍历；先缓存将要添加的元素
	if (bPersistentTickTasksLooping)
	{
		PersistentTickTasksToAdd.Add(Task);

		return;
	}
	PersistentTickTasks.AddUnique(Task);

	// 尝试把自己从上一个MissionManager中添加持续Tick（避免当前不是Tick状态，突然有一个子任务要激活Tick）TODO: 优化此注册（判断自己非Tick时才注册自己）
	if (MissionManagerPtr)
	{
		MissionManagerPtr->RegisterPersistentTick(this);
	}
}

void UTask_Parallel::UnRegisterPersistentTick(UTaskObject* Task)
{
	// 当前数组正在遍历；先缓存将要移除的元素
	if (bPersistentTickTasksLooping)
	{
		int32 Index = PersistentTickTasks.Find(Task);
		if (Index != INDEX_NONE)
			PersistentTickTaskRemoveIndices.AddUnique(Index);

		return;
	}

	PersistentTickTasks.Remove(Task);

	// SubMission没有可以Tick的了，把自己从上一个MissionManager中移除持续Tick
	if (PersistentTickTasks.Num() == 0 && MissionManagerPtr)
	{
		MissionManagerPtr->UnRegisterPersistentTick(this);
	}
}

bool UTask_Parallel::GetHasPersistenTickTask()
{
	return PersistentTickTasks.Num() > 0;
}

void UTask_Parallel::ClearTask()
{
	Super::ClearTask();

	// 清空PersistentTick相关数组
	PersistentTickTasks.Empty();
	PersistentTickTasksToAdd.Empty();
	PersistentTickTaskRemoveIndices.Empty();

	// 子任务基本是同步执行对于下一帧来说，所有都做Clear
	for (auto sm : SubMissions)
	{
		sm->ClearTask();
	}

	// 清空子任务相关数组
	SubMissions.Empty();
	FinishedSubMissions.Empty();
}


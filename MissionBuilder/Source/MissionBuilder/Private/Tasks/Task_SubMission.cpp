


#include "Tasks/Task_SubMission.h"
#include "TaskList.h"
#include "MissionManager.h"

UTask_SubMission::UTask_SubMission()
{
	TaskName = FText::FromString(TEXT("子任务"));
	bPersistentTick = true;

#if WITH_EDITORONLY_DATA
	Note = TEXT("");
	NodeColor = FLinearColor(0.8f, 0.f, 0.8f);
#endif
}

void UTask_SubMission::ExecuteTask(UObject* MissionManagerObj)
{
	Super::ExecuteTask(MissionManagerObj);

	if (MissionObject == nullptr && MissionObjectAsset)
	{
		MissionObject = DuplicateObject(MissionObjectAsset, this);
	}

	// 根任务不存在将不执行
	if (!MissionObject || MissionObject->RootTask == nullptr)
		return;

	UE_LOG(LogMissionManager, Log, TEXT("开始执行子任务: %s"), *MissionObject->GetName());
	// 清空执行列表
	TaskExecuteList.Empty();
	// 设置当前任务
	CurrentTask = MissionObject->RootTask;

	// 开始执行Task
	ExecuteTask();
}

void UTask_SubMission::AbortTask()
{
	Super::AbortTask();

	// 中止当前Task
	if (CurrentTask && CurrentTask->IsEnable())
		CurrentTask->AbortTask();

	// 重置Tasks - 包括CurrentTask也会重置
	ResetTask();
}

void UTask_SubMission::ResetTask()
{
	Super::ResetTask();

	// 执行列表为空将不执行
	if (TaskExecuteList.Num() == 0)
		return;

	// 根据执行顺序缓存的列表来重置
	for (int32 i = TaskExecuteList.Num() - 1; i >= 0; --i)
	{
		// 未开启的不管
		if(TaskExecuteList[i]->IsEnable())
			TaskExecuteList[i]->ResetTask();
	}
}

void UTask_SubMission::ClearTask()
{
	Super::ClearTask();

	DestroyCurrentMission();
}

void UTask_SubMission::SkipTask(UObject* MissionManagerObj)
{
	Super::SkipTask(MissionManagerObj);

	if (MissionObject == nullptr && MissionObjectAsset)
	{
		// 从赋值的资源中赋值一份，因为需要独立的实例来存储独立的数据
		MissionObject = DuplicateObject(MissionObjectAsset, this);
	}

	// 根任务不存在将不执行
	if (MissionObject || MissionObject->RootTask == nullptr)
		return;

	// 清空执行列表
	TaskExecuteList.Empty();

	CurrentTask = MissionObject->RootTask;
	// 执行跳过所有子Tasks
	UTaskObject* TaskToSkip = CurrentTask;
	while (TaskToSkip)
	{
		// 未开启就忽略
		if (!TaskToSkip->IsEnable())
		{
			CurrentTask = TaskToSkip;
			continue;
		}

		TaskToSkip->SkipTask(this);
		TaskToSkip = TaskToSkip->GetNextTask();
		if (TaskToSkip)
		{
			// 更新CurrentTask
			CurrentTask = TaskToSkip;
			// 添加到执行列表
			TaskExecuteList.Add(TaskToSkip);
		}
	}
}

void UTask_SubMission::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 当前任务是否已经执行过Tick了
	bool IsCurrentTaskTicked = false;

	// 开始遍历
	bPersistentTickTasksLooping = true;
	// Tick持续性Tick的Task
	for (auto TickTask : PersistentTickTasks)
	{
		TickTask->Tick(DeltaTime);

		if (TickTask == CurrentTask)
			IsCurrentTaskTicked = true;
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

		// 子Task没有可以Tick的了，把自己从父MissionManager中移除持续Tick
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

	if (!CurrentTask || IsCurrentTaskTicked)
		return;

	// Tick当前Task
	CurrentTask->Tick(DeltaTime);
}

void UTask_SubMission::DestroyCurrentMission()
{
	if (!MissionObject)
		return;

	// 清空PersistentTick相关数组
	PersistentTickTasks.Empty();
	PersistentTickTasksToAdd.Empty();
	PersistentTickTaskRemoveIndices.Empty();

	for (int32 i = TaskExecuteList.Num() - 1; i >= 0; --i)
	{
		auto Task = TaskExecuteList[i];
		if (Task && Task->IsEnable())
		{
			Task->ClearTask();
		}
	}

	// TODO: 删除旧的Mission所有TaskObject

	// 清空指针
	MissionObject = nullptr;
	CurrentTask = nullptr;

}

void UTask_SubMission::OnSubMissionComplete()
{
	FinishExecute(true);
}

void UTask_SubMission::OnTaskComplete(bool Success)
{
	// 是否成功
	if (Success)
	{
		// 执行下一个任务
		ToNextTask();
	}
	else
	{
		// 是否需要失败后再次执行
		if (CurrentTask->bReRunOnFalied)
		{
			CurrentTask->ExecuteTask(this);
		}
	}
}

void UTask_SubMission::ToNextTask()
{
	// 是否没有Task了
	if (!CurrentTask->GetNextTask())
	{	
		// 子任务结束
		OnSubMissionComplete();
		return;
	}

	// 获取下一个Task
	CurrentTask = CurrentTask->GetNextTask();
	// 判断是否可以执行，禁用执行的话，接着看下一个任务
	if (!CurrentTask->IsEnable())
	{
		ToNextTask();
		return;
	}

	// 执行Task
	ExecuteTask();
}

// TODO: 重构此函数或者移除
void UTask_SubMission::JumpToTask(int32 TaskIDToJump)
{
	// 寻找给定Task的索引
	int32 TaskIndex = TaskExecuteList.IndexOfByPredicate([TaskIDToJump](const UTaskObject* Task) {return TaskIDToJump == Task->TaskID; });
	if (TaskIndex == INDEX_NONE)
	{
		UE_LOG(LogMissionManager, Error, TEXT("SubMission 无法跳到任务，指定任务不存在，ID: %d"), TaskIDToJump);
		return;
	}

	// TODO: 此Index在该模式可能不能执行，向下寻找可以执行Task

	CurrentTaskIndex = TaskIndex;
	// 开始执行任务
	ExecuteTask();
}

// TaskObject执行结束会调用此函数
void UTask_SubMission::OnTaskFinished(UTaskObject* Task, bool bSuccess)
{
	// 刚刚结束的任务为当前执行的任务才继续（避免Task中意外调用FinidhTask()触发此函数）
	if (CurrentTask != Task)
		return;

	OnTaskComplete(bSuccess);
}

APawn* UTask_SubMission::GetPlayerPawn()
{
	return nullptr;
}

FString UTask_SubMission::GetMissionName()
{
	FString OutName;
	if (MissionObject)
	{
		if (!MissionObject->MissionName.IsEmpty())
			OutName = MissionObject->MissionName;
		else
			OutName = MissionObject->GetFName().ToString();
	}

	return OutName;
}

void UTask_SubMission::RegisterPersistentTick(UTaskObject* Task)
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

void UTask_SubMission::UnRegisterPersistentTick(UTaskObject* Task)
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

	// 子Task没有可以Tick的了，把自己从父MissionManager中移除持续Tick
	if (PersistentTickTasks.Num() == 0 && MissionManagerPtr)
	{
		MissionManagerPtr->UnRegisterPersistentTick(this);
	}
}

bool UTask_SubMission::GetHasPersistenTickTask()
{
	return PersistentTickTasks.Num() > 0;
}

void UTask_SubMission::SetMissionObject(UMissionObject* MissionObj)
{
	MissionObject = MissionObj;
}

void UTask_SubMission::ExecuteTask()
{
	if (CurrentTask)
	{
		// 先添加到执行列表
		TaskExecuteList.Add(CurrentTask);
		// 再执行Task
		CurrentTask->ExecuteTask(this);
	}
}

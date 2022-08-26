


#include "MissionManager.h"
#include "TaskList.h"
#include "MissionObject.h"
#include "TaskObject.h"
#include "Tasks/Task_Branch.h"

DEFINE_LOG_CATEGORY(LogMissionManager);


UMissionManager::UMissionManager() : MissionList(nullptr), CurrentMissionIndex(-1), CurrentTaskIndex(-1)/*, CurrentTrainMode(ETrainMode::Learn)*/
{

}

void UMissionManager::SetMissionObject(UMissionObject* NewMission)
{
	if (!NewMission)
		return;

	/**任务开始检查缓存里是否有之前任务生成的Actor销毁生成成功的Actor*/
	DestroyCurrentMission();

	// 复制一份使用
	Mission = DuplicateObject(NewMission, this);

	
}

UMissionObject* UMissionManager::GetMissionObject()
{
	if (Mission)
	{
		return Mission;
	}
	else
	{
	    return nullptr;
	}
}

void UMissionManager::DestroyCurrentMission()
{
	if (!Mission)
		return;

	// 清空PersistentTick相关数组
	PersistentTickTasksToAdd.Empty();
	PersistentTickTaskRemoveIndices.Empty();
	PersistentTickTasks.Empty();

	// 遍历所有执行过的Tasks，调用ClearTask
	for (int32 i = TaskExecuteList.Num() - 1; i >= 0; --i)
	{
		auto Task = TaskExecuteList[i];
		if (Task)
		{
			Task->ClearTask();
		}
	}
	// 清空执行过的Tasks
	TaskExecuteList.Empty();

	// TODO: 删除旧的Mission所有TaskObject

	Mission = nullptr;
	CurTask = nullptr;
	CurMission = nullptr;

}

UTaskList* UMissionManager::GetMissionList()
{
	return MissionList;
}

void UMissionManager::StartMission(int32 TaskIDToStart)
{
	if (!Mission)
	{
		UE_LOG(LogMissionManager, Error, TEXT("任务不存在，无法执行任务"));
		return;
	}

	if (!Mission->RootTask)
	{
		UE_LOG(LogMissionManager, Error, TEXT("任务列表为空，无法执行任务"));
		return;
	}

	CurTask = FindTaskByID(TaskIDToStart);
	if (CurTask == nullptr)
	{
		CurTask = Mission->RootTask;
		UE_LOG(LogMissionManager, Warning, TEXT("没找到对应ID %d 的任务，将从第0个任务开始"), TaskIDToStart);
	}

	UE_LOG(LogMissionManager, Log, TEXT("开始执行任务: %s"), *Mission->GetName());
	// 开始执行任务
	ExecuteMission();
}

void UMissionManager::Next()
{
	//if (!MissionList->Missions.IsValidIndex(CurrentMissionIndex))
	//{
	//	// 没有任务了，任务结束
	//	UE_LOG(LogMissionManager, Warning, TEXT("无法执行下一个任务， 当前指向任务不存在了"));
	//	// 当前没有可执行任务
	//	bAllMissionComplete = true;

	//	return;
	//}

	//// 获取当前任务
	//CurMission = MissionList->Missions[CurrentMissionIndex];

	//	if (CurMission == nullptr)
	//{
	//	UE_LOG(LogMissionManager, Error, TEXT("无法执行下一个Task，任务列表中MissionObject为空，列表索引：%d"), CurrentMissionIndex);
	//	return;
	//}

	//// 获取下一个任务有效Index
	//int32 NextValidTaskIndex = GetNextValidTaskIndex(CurrentTaskIndex + 1, CurMission->Tasks);
	//// 下一个子任务不存在
	//if (!CurMission->Tasks.IsValidIndex(NextValidTaskIndex))
	//{
	//	int32 NextValidMissionIndex = INDEX_NONE;
	//	int32 NextValidMissionTaskIndex = INDEX_NONE;
	//	bool bNextMissionValid = GetNextValidTaskIndex(0, CurrentMissionIndex + 1, NextValidMissionTaskIndex, NextValidMissionIndex);
	//	// 下一个主任务不存在(或者以后任务没有有效Task)
	//	if (!MissionList->Missions.IsValidIndex(CurrentMissionIndex + 1) || !bNextMissionValid)
	//	{
	//		// 任务完成回调
	//		OnAllMissionComplete.Broadcast();

	//		// 没有任务了，任务结束
	//		UE_LOG(LogMissionManager, Warning, TEXT("无法执行下一个任务， 下一个指向任务不存在了"));
	//		// 当前没有可执行任务
	//		bAllMissionComplete = true;

	//		return;
	//	}

	//	// 切换下一个主任务
	//	CurrentMissionIndex = NextValidMissionIndex;
	//	CurrentTaskIndex = NextValidMissionTaskIndex; // 设置下一个有效Index

	//	// 任务更新回调 -- 结束执行任务
	//	OnMissionUpdate.Broadcast(false, CurMission);
	//}
	//else
	//{
	//	// 切换下一个子任务
	//	CurrentTaskIndex = NextValidTaskIndex;
	//}

	// 是否没有Task了
	if (!CurTask->GetNextTask())
	{
		// 任务完成回调
		OnAllMissionComplete.Broadcast();

		// 没有任务了，任务结束
		UE_LOG(LogMissionManager, Warning, TEXT("无法执行下一个任务， 下一个指向任务不存在了"));
		// 当前没有可执行任务
		bAllMissionComplete = true;
		return;
	}

	// 获取下一个Task
	CurTask = CurTask->GetNextTask();
	// 判断是否可以执行，禁用执行的话，接着看下一个任务
	if (!CurTask->IsEnable())
	{
		Next();
		return;
	}

	// 执行任务
	ExecuteMission();
}

void UMissionManager::RevertMission()
{
	if (CurrentMissionIndex < 0 || CurrentTaskIndex < 0)
		return;

	int32 NewMissionIndex = 0;

	int32 LastTaskID = FMath::Max(0, CurrentTaskIndex - 1);
	UMissionObject* Mission_ = CurMission ? CurMission : MissionList->Missions[CurrentMissionIndex];

	// 回滚前回调
	OnPreMissionRevert.Broadcast(Mission_);

	// 还有任务在执行
	if (!bAllMissionComplete)
	{
		// 中止当前Task
		CurTask->AbortTask();
	}

	// 回滚当前大任务
	RevertMission_Internal(Mission_, LastTaskID);

	/// TODO: 向上滚到一个可以有效Task的任务
	// 设置上一个大任务ID
	GetPreviousValidMissionIndex(CurrentMissionIndex, NewMissionIndex);

	// 上一任务不是第一个任务(TODO: 第0个任务可能没有可执行的Task)
	if (NewMissionIndex >= 0)
	{
		UMissionObject* NewMission = MissionList->Missions[NewMissionIndex];
		// 重置整个任务
		RevertMission_Internal(NewMission, -1);
	}

	CurrentMissionIndex = NewMissionIndex;
	// 获取下一个有效Task
	CurrentTaskIndex = GetNextValidTaskIndex(0, MissionList->Missions[NewMissionIndex]->Tasks);

	// 任务回退广播
	OnMissionRevert.Broadcast(MissionList->Missions[CurrentMissionIndex]);

	// 开始执行任务
	ExecuteMission();

	//// 开始执行任务
	//BeginMission(NewMissionIndex);
}

void UMissionManager::RevertToMission(int32 NewMissionID)
{
	int32 MissionIndex = -1;
	if (!GetMissionIndexByID(NewMissionID, MissionIndex))
	{
		return;
	}

	// 回滚到之后的任务，不执行
	if (CurrentMissionIndex < MissionIndex)
		return;

	if (CurrentMissionIndex < 0 || CurrentTaskIndex < 0)
		return;

	int32 NewMissionIndex = 0;

	int32 LastTaskID = FMath::Max(0, CurrentTaskIndex - 1);
	UMissionObject* Mission_ = CurMission ? CurMission : MissionList->Missions[CurrentMissionIndex];
	// 回滚前回调
	OnPreMissionRevert.Broadcast(Mission_);

	// 还有任务在执行
	if (!bAllMissionComplete)
	{
		// 中止当前Task
		CurTask->AbortTask();
	}

	// 回滚当前大任务
	RevertMission_Internal(Mission_, LastTaskID);

	/// TODO: 向上滚到一个可以有效Task的任务
	// 设置上一个大任务ID
	GetPreviousValidMissionIndex(CurrentMissionIndex, NewMissionIndex);

	// 回滚到当前任务之前
	if (CurrentMissionIndex > MissionIndex)
	{
		// 回滚主任务
		for (int32 RevertMissionIndex = CurrentMissionIndex - 1; RevertMissionIndex >= MissionIndex; --RevertMissionIndex)
		{
			UMissionObject* NewMission = MissionList->Missions[RevertMissionIndex];
			// 重置整个任务
			RevertMission_Internal(NewMission, -1);
		}
	}

	CurrentMissionIndex = MissionIndex;
	// 任务回退广播
	OnMissionRevert.Broadcast(MissionList->Missions[CurrentMissionIndex]);
	// 获取下一个有效Task
	CurrentTaskIndex = GetNextValidTaskIndex(0, MissionList->Missions[NewMissionIndex]->Tasks);

	// 开始执行任务
	ExecuteMission();
}

void UMissionManager::SkipToTask(int32 NewTaskID)
{
	if (!CurTask)
	{
		if (!Mission)
		{
			UE_LOG(LogMissionManager, Error, TEXT("任务不存在，无法执行任务"));
			return;
		}

		if (!Mission->RootTask)
		{
			UE_LOG(LogMissionManager, Error, TEXT("任务列表为空，无法执行任务"));
			return;
		}

		if (CurTask == nullptr)
		{
			CurTask = Mission->RootTask;
		}
	}

	// 直到任务完成或者遇到指定ID才停止跳步
	while (CurTask && CurTask->TaskID < NewTaskID && !bAllMissionComplete)
	{
		// 该任务开启执行
		if (CurTask->IsEnable())
		{
			// 添加到执行列表
			TaskExecuteList.Add(CurTask);
			CurTask->SkipTask(this);
		}

		// 获取下个任务
		CurTask = CurTask->GetNextTask();
	}

	// 继续执行其他Task
	ExecuteMission();
}

void UMissionManager::SkipToMission(int32 NewMissionID)
{
	int32 MissionIndex = -1;
	if (!GetMissionIndexByID(NewMissionID, MissionIndex))
	{
		return;
	}

	// 跳到当前任务或之前任务，不执行
	if (CurrentMissionIndex >= MissionIndex)
		return;

	// 任务跳步前回调
	OnPreMissionSkip.Broadcast(CurMission);

	// 中止当前Task
	CurTask->AbortTask();
	// 跳过当前任务
	SkipMission_Internal(CurMission, CurrentTaskIndex);

	// 其余任务跳过
	for (int32 SkipMissionIndex = CurrentMissionIndex + 1; SkipMissionIndex < MissionIndex; ++SkipMissionIndex)
	{
		SkipMission_Internal(MissionList->Missions[SkipMissionIndex], 0);
	}

	CurrentMissionIndex = MissionIndex;
	// 获取下一个有效Task
	CurrentTaskIndex = GetNextValidTaskIndex(0, MissionList->Missions[MissionIndex]->Tasks);

	// 任务跳步后回调
	OnPostMissionSkip.Broadcast(MissionList->Missions[MissionIndex]);

	// 开始执行任务
	ExecuteMission();
}

void UMissionManager::SwitchToMission(int32 MissionID)
{
	int32 MissionIndex = -1;
	if (!GetMissionIndexByID(MissionID, MissionIndex))
	{
		UE_LOG(LogMissionManager, Error, TEXT("无法切换主任务， 任务不存在: %d"), MissionID);
		return;
	}

	if (MissionIndex > CurrentMissionIndex)
	{
		SkipToMission(MissionID);
	}
	else
	{
		RevertToMission(MissionID);
	}
}

void UMissionManager::ToNextTask()
{
	Next();
}

void UMissionManager::JumpToTask(int32 TaskIDToJump)
{
	JumpToTask_Internal(TaskIDToJump);
}

void UMissionManager::JumpToTask_Internal(int32 TaskID)
{
	// 寻找给定Task的索引
	int32 TaskIndex = CurMission->Tasks.IndexOfByPredicate([TaskID](const UTaskObject* Task) {return TaskID == Task->TaskID; });
	if (TaskIndex == INDEX_NONE)
	{
		UE_LOG(LogMissionManager, Error, TEXT("无法跳到任务，指定任务不存在，ID: %d"), TaskID);
		return;
	}

	// 此Index在该模式可能不能执行，向下寻找可以执行Task
	int32 OutTaskIndex, OutMissionIndex;
	GetNextValidTaskIndex(TaskIndex, CurrentMissionIndex, OutTaskIndex, OutMissionIndex);

	CurrentMissionIndex = OutMissionIndex;
	CurrentTaskIndex = OutTaskIndex;
	// 开始执行任务
	ExecuteMission();
}

UTaskObject* UMissionManager::FindTaskByID(int32 TaskID)
{
	if (!Mission || !Mission->RootTask)
		return nullptr;

	UTaskObject* OutTask = nullptr;
	FindTaskByID(TaskID, Mission->RootTask, OutTask);

	return OutTask;
}

void UMissionManager::FindTaskByID(int32 TaskID, UTaskObject* FirstTask, UTaskObject*& OutTask)
{
	if (!FirstTask)
		return;

	if (FirstTask->TaskID == TaskID)
	{
		OutTask = FirstTask;
		return;
	}

	if (UTask_Branch* Branch = Cast<UTask_Branch>(FirstTask))
	{
		// 分支中查找 - 遇到左右分支连接相同的Task时跳出（代表分支结束）
		UTaskObject* BranchEndTask = FindTaskInBranchByID(TaskID, Branch, OutTask);
		// 已经找到返回
		if (OutTask != nullptr)
		{
			return;
		}

		FindTaskByID(TaskID, BranchEndTask, OutTask);
	}
	else
	{
		if (FirstTask->NextTask && FirstTask->NextTask->TaskID == TaskID)
		{
			OutTask = FirstTask->NextTask;
			return;
		}
		else
		{
			FindTaskByID(TaskID, FirstTask->NextTask, OutTask);
		}
	}
}

UTaskObject* UMissionManager::FindTaskInBranchByID(int32 TaskID, class UTask_Branch* Branch, UTaskObject*& OutTask)
{
	bool LeftTurn = true;

	UTaskObject* CurLeft = Branch->NextTask;
	UTaskObject* CurRight = Branch->NextTask2;

	UTaskObject* EndBranch = nullptr;

	// 遇到左右分支连接相同的Task时跳出（代表分支结束）==》左边遍历一个节点，再接着右边遍历一个节点，一层层往下（遇到子分支则整个分支看成一个节点）
	while (CurLeft != CurRight)
	{
		if (LeftTurn)
		{
			// 左边Task为空
			if (!CurLeft)
			{
				LeftTurn = false; // 下一轮操作右边
			}
			else
			{
				EndBranch = CurLeft;

				if (CurLeft->TaskID == TaskID)
				{
					OutTask = CurLeft;
					return OutTask;
				}

				if (UTask_Branch* LeftBranch = Cast<UTask_Branch>(CurLeft))
				{
					UTaskObject* BranchEndTask = FindTaskInBranchByID(TaskID, LeftBranch, OutTask);
					// 已经找到返回
					if (OutTask != nullptr)
					{
						return OutTask;
					}

					CurLeft = BranchEndTask; // 更新左边节点
					LeftTurn = false;	// 轮到右边判断

					if (CurLeft != nullptr)
						EndBranch = CurLeft;
				}
				else
				{				
					CurLeft = CurLeft->NextTask; // 更新左边节点
					LeftTurn = false;	// 轮到右边判断

					if (CurLeft != nullptr)
						EndBranch = CurLeft;
				}
			}
		}
		else
		{
			// 右边Task为空
			if (!CurRight)
			{
				LeftTurn = true; // 下一轮操作左边
			}
			else
			{
				EndBranch = CurRight;

				if (CurRight->TaskID == TaskID)
				{
					OutTask = CurRight;
					return OutTask;
				}

				if (UTask_Branch* RightBranch = Cast<UTask_Branch>(CurRight))
				{
					UTaskObject* BranchEndTask = FindTaskInBranchByID(TaskID, RightBranch, OutTask);
					// 已经找到返回
					if (OutTask != nullptr)
					{
						return OutTask;
					}

					CurRight = BranchEndTask; // 更新右边节点
					LeftTurn = true;	// 轮到左边判断

					if (CurRight != nullptr)
						EndBranch = CurRight;
				}
				else
				{
					CurRight = CurRight->NextTask; // 更新右边节点
					LeftTurn = true;	// 轮到左边判断

					if (CurRight != nullptr)
						EndBranch = CurRight;
				}
			}
		}
	}

	return EndBranch;
}

void UMissionManager::OnTaskFinished(UTaskObject* Task, bool bSuccess)
{
	// 刚刚结束的任务为当前执行的任务才继续（避免Task中意外调用FinidhTask()触发此函数）
	if (CurTask != Task)
		return;


	// 是否成功
	if (bSuccess)
	{
		// 执行下一个任务
		ToNextTask();
	}
	else
	{
		// 是否需要失败后再次执行
		if (CurTask->bReRunOnFalied)
		{
			ExecuteMission();
		}
	}
}

APawn* UMissionManager::GetPlayerPawn()
{
	return PlayerPawn;
}

FString UMissionManager::GetMissionName()
{
	FString OutName;
	if (Mission)
	{
		if (!Mission->MissionName.IsEmpty())
			OutName = Mission->MissionName;
		else
			OutName = Mission->GetFName().ToString();
	}

	return OutName;
}

void UMissionManager::RegisterPersistentTick(UTaskObject* Task)
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
}

void UMissionManager::UnRegisterPersistentTick(UTaskObject* Task)
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
}

//ETrainMode UMissionManager::GetTrainMode()
//{
//	return CurrentTrainMode;
//}

void UMissionManager::Tick(float DeltaTime)
{
	// 当前任务是否已经执行过Tick了
	bool IsCurrentTaskTicked = false;

	// 开始遍历
	bPersistentTickTasksLooping = true;
	// Tick持续性Tick的Task
	for (auto TickTask : PersistentTickTasks)
	{
		TickTask->Tick(DeltaTime);

		if (TickTask == CurTask)
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
	}
	// 存在将要添加的Task到PersistentTickTasks
	if (PersistentTickTasksToAdd.Num() > 0)
	{
		for (auto i : PersistentTickTasksToAdd)
		{
			PersistentTickTasks.AddUnique(i);
		}
		PersistentTickTasksToAdd.Empty();
	}

	// Tick当前Task
	if (!bAllMissionComplete && CurTask && !IsCurrentTaskTicked)
	{
		CurTask->Tick(DeltaTime);
	}
}

void UMissionManager::ExecuteMission()
{
	// 当前有可执行任务
	bAllMissionComplete = false;

	//if (!MissionList->Missions.IsValidIndex(CurrentMissionIndex))
	//{
	//	// 没有任务了，任务结束
	//	UE_LOG(LogMissionManager, Warning, TEXT("无法执行任务， 当前指向任务不存在了"));

	//	// 当前没有可执行任务
	//	bAllMissionComplete = true;
	//	return;
	//}

	//// 获取当前任务
	//CurMission = MissionList->Missions[CurrentMissionIndex];

	// 任务更新回调 -- 开始执行任务前（已更新CurMission)
	OnMissionUpdate.Broadcast(true, CurMission);

	//if (!CurMission->Tasks.IsValidIndex(CurrentTaskIndex))
	//{
	//	// 没有任务了，任务结束
	//	UE_LOG(LogMissionManager, Warning, TEXT("无法执行子任务， 当前指向子任务不存在了"));

	//	// 当前没有可执行任务
	//	bAllMissionComplete = true;
	//	return;
	//}

	//// 大任务改变
	//if (OldMissionIndex != CurrentMissionIndex)
	//{
	//	OldMissionIndex = CurrentMissionIndex;
	//	UE_LOG(LogMissionManager, Warning, TEXT("执行Mission: %s | Id = %d"), *CurMission->MissionName.ToString(), CurMission->MissionID);
	//}

	//// 获取当前子任务
	//CurTask = CurMission->Tasks[CurrentTaskIndex];

	// 执行子任务
	if (CurTask)
	{
		// 添加到执行列表
		TaskExecuteList.Add(CurTask);
		CurTask->ExecuteTask(this);
	}
}

void UMissionManager::RevertMission_Internal(UMissionObject* Mission_, int32 LastTaskIndex)
{
	int Index = LastTaskIndex < 0 ? Mission_->Tasks.Num() - 1 : LastTaskIndex;

	// 回滚当前大任务
	for (int32 i = Index; i >= 0; --i)
	{
		// 当前模式可以执行的重置
		//if (Mission->Tasks[i]->AllowMode & int32(CurrentTrainMode))
		if (Mission_->Tasks[i]->IsEnable())
		{
			// 执行任务重置
			Mission_->Tasks[i]->ResetTask();
		}
		else
		{
			UE_LOG(LogMissionManager, Log, TEXT("当前任务无法重置Task: %s"), *Mission_->Tasks[i]->GetName());
		}
	}

}

void UMissionManager::SkipMission_Internal(UMissionObject* Mission_, int32 FirstTaskIndex)
{
	// 回滚当前大任务
	for (int32 i = FirstTaskIndex; i < Mission_->Tasks.Num(); ++i)
	{
		// 当前模式可以执行的Task
		if (Mission_->Tasks[i] && Mission_->Tasks[i]->IsEnable())// & int32(CurrentTrainMode))
		{
			// 执行任务重置
			Mission_->Tasks[i]->SkipTask(this);
		}
	}
}

int32 UMissionManager::GetNextValidTaskIndex(int32 _CurrentTaskIndex, const TArray<UTaskObject*>& Tasks)
{
	if (!Tasks.IsValidIndex(_CurrentTaskIndex))
		return INDEX_NONE;

	const UTaskObject* Task = Tasks[_CurrentTaskIndex];

	if (Task->IsEnable())// & int32(CurrentTrainMode))
		return _CurrentTaskIndex;
	else
		return GetNextValidTaskIndex(++_CurrentTaskIndex, Tasks);
}

bool UMissionManager::GetNextValidTaskIndex(int32 InTaskIndex, int32 InMissionIndex, int32& OutTaskIndex, int32& OutMissionIndex)
{
	if (!MissionList->Missions.IsValidIndex(InMissionIndex))
		return false;

	const UMissionObject* Mission_ = MissionList->Missions[InMissionIndex];
	OutTaskIndex = GetNextValidTaskIndex(InTaskIndex, Mission_->Tasks);
	if (OutTaskIndex != INDEX_NONE)
	{
		OutMissionIndex = InMissionIndex;
		return true;
	}

	return GetNextValidTaskIndex(0, ++InMissionIndex, OutTaskIndex, OutMissionIndex);
}

bool UMissionManager::GetPreviousValidTaskIndex(int32 InTaskIndex, int32 InMissionIndex, int32& OutTaskIndex, int32& OutMissionIndex)
{
	return false;
}

bool UMissionManager::GetPreviousValidMissionIndex(int32 _CurrentMissionIndex, int32& OutMissionIndex)
{
	OutMissionIndex = FMath::Max(0, _CurrentMissionIndex - 1);
	return true;
}

bool UMissionManager::GetMissionIndexByID(int32 MissionID, int32& OutMissionIndex)
{
	int32 MissionIndex = 0;// MissionList->Missions.IndexOfByPredicate([MissionID](const UMissionObject* Mission) {return Mission->MissionID == MissionID; });
	if (MissionIndex == INDEX_NONE)
		return false;

	OutMissionIndex = MissionIndex;
	return true;
}

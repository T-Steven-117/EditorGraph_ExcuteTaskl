

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
//#include "../Defines/CommonDefines.h"
#include "Tickable.h"
#include "MissionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MissionManager.generated.h"

class UTaskList;
template<class TClass>
class TSubclassOf;
class UTaskObject;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMissionUpdate, bool, IsMissionBegin, UMissionObject*, CurrentMission);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionRevert, UMissionObject*, CurrentMission);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllMissionComplete);

/**
 * 
 */
UCLASS()
class MISSIONBUILDER_API UMissionManager : public UGameInstanceSubsystem, public FTickableGameObject, public IMissionInterface
{
	GENERATED_BODY()

public:
	UMissionManager();

	/** 设置要执行的任务 */
	UFUNCTION(BlueprintCallable, Category = "MissionManager")
		void SetMissionObject(UMissionObject* NewMission);

	UFUNCTION(BlueprintCallable, Category = "MissionManager")
		UMissionObject* GetMissionObject();

	/** 获取已设置的任务列表  */
	UFUNCTION(BlueprintPure, Category = "MissionManager")
		UTaskList* GetMissionList();

	/** 开始执行任务，可以设置起始执行的任务ID */
	UFUNCTION(BlueprintCallable, Category = "MissionManager")
		void StartMission(int32 TaskIDToStart);

	/** 执行下一个子任务 */
	UFUNCTION(BlueprintCallable, Category = "MissionManager")
		void Next();

	/** 后退一个大任务 */
	UFUNCTION(BlueprintCallable, Category = "MissionManager")
		void RevertMission();

	/** 回滚到新的任务执行 */
	void RevertToMission(int32 NewMissionID);

	/** 任务跳到指定TaskID执行；NOTE:如果该ID在Branch里面，Branch又没执行该分支，任务会执行到结束 */
	UFUNCTION(BlueprintCallable, Category = "MissionManager")
	void SkipToTask(int32 NewTaskID);

	/** 跳到新的任务执行 */
		void SkipToMission(int32 NewMissionID);

	/** 切换到指定任务（引发回滚或者快进） */
	UFUNCTION(BlueprintCallable, Category = "MissionManager")
		void SwitchToMission(int32 MissionID);

	//UFUNCTION(BlueprintCallable, Category = "MissionManager")
	//ETrainMode GetTrainMode();

	/** TickableGameObject Interface */
	virtual TStatId GetStatId() const override { return Super::GetStatID(); }

	virtual void Tick(float DeltaTime) override;

	virtual bool IsTickable() const override { return true; }

	/** TickableGameObject Interface End */

	/************ Begin IMissionInterafce  ***********/
	virtual void ToNextTask() override;

	/** 当前Mission内跳到指定Task */
	virtual void JumpToTask(int32 TaskIDToJump) override;

	/** 子任务完成回调 */
	//UFUNCTION(BlueprintCallable, Category = "MissionManager")
	virtual void OnTaskFinished(UTaskObject* Task, bool bSuccess) override;

	//UFUNCTION(BlueprintCallable, Category = "MissionManager")
	virtual APawn* GetPlayerPawn() override;

	/** 获取任务名称 */
	virtual FString GetMissionName() override;

	/** 注册持续性Tick；任务开始执行后就算FinishTask后还会Tick - 除非调用UnRegister */
	virtual void RegisterPersistentTick(UTaskObject* Task) override;
	/** 取消持续性Tick */
	virtual void UnRegisterPersistentTick(UTaskObject* Task) override;


	/************ End IMissionInterafce  ***********/
public:
	/** 任务更新回调 */
	UPROPERTY(BlueprintAssignable, Category = "MissionManager")
		FOnMissionUpdate OnMissionUpdate;

	/** 全部任务完成回调 */
	UPROPERTY(BlueprintAssignable, Category = "MissionManager")
		FOnAllMissionComplete OnAllMissionComplete;

	/** 任务返回回调 - 执行完了Abort,ResetTask的过程 */
	UPROPERTY(BlueprintAssignable, Category = "MissionManager")
		FOnMissionRevert OnMissionRevert;
	/** 任务返回前期回调 - 准备执行任务回滚,还未执行Abort,ResetTask的过程 */
	UPROPERTY(BlueprintAssignable, Category = "MissionManager")
		FOnMissionRevert OnPreMissionRevert;

	/** 任务跳步前回调 - 还未执行Abort,SkipTask的过程 */
	UPROPERTY(BlueprintAssignable, Category = "MissionManager")
		FOnMissionRevert OnPreMissionSkip;
	/** 任务跳步后回调 - 执行完了Abort,SkipTask的过程,准备开始执行新任务 */
	UPROPERTY(BlueprintAssignable, Category = "MissionManager")
		FOnMissionRevert OnPostMissionSkip;

protected:
	/** 销毁上一次执行的Mission */
	void DestroyCurrentMission();

	void ExecuteMission();

	/** 重置整个主任务
	* @param	Mission			给定主任务
	* @param	LastTaskIndex	最后的TaskIndex（-1的话为整个任务都重置）
	*/
	void RevertMission_Internal(UMissionObject* Mission, int32 LastTaskIndex);

	/** 跳过整个主任务
	* @param	Mission			给定主任务
	* @param	LastTaskIndex	开始的TaskIndex
	*/
	void SkipMission_Internal(UMissionObject* Mission, int32 FirstTaskIndex);

	/** 当前Mission内跳到指定Task */
	//UFUNCTION(BlueprintCallable, Category = "MissionManager")
	void JumpToTask_Internal(int32 TaskID);

	UTaskObject* FindTaskByID(int32 TaskID);

	void FindTaskByID(int32 TaskID, UTaskObject* FirstTask, UTaskObject*& OutTask);

	UTaskObject* FindTaskInBranchByID(int32 TaskID, class UTask_Branch* Branch, UTaskObject*& OutTask);

private:
	/** 给定一个Index， 获取列表中有效（当前模式可以执行）的下一个Index(也包括当前的检查） */
	int32 GetNextValidTaskIndex(int32 _CurrentTaskIndex, const TArray<UTaskObject*>& Tasks);
	/** 给定一个Task Index，和Mission Index 获取任务列表中有效（当前模式可以执行）的下一个Task Index和所在的Mission Index(也包括当前的检查） */
	bool GetNextValidTaskIndex(int32 InTaskIndex, int32 InMissionIndex, int32& OutTaskIndex, int32& OutMissionIndex);
	/** 给定一个Task Index，和Mission Index 获取任务列表中有效（当前模式可以执行）的上一个Task Index和所在的Mission Index(也包括当前的检查） */
	bool GetPreviousValidTaskIndex(int32 InTaskIndex, int32 InMissionIndex, int32& OutTaskIndex, int32& OutMissionIndex);

	bool GetPreviousValidMissionIndex(int32 _CurrentMissionIndex, int32& OutMissionIndex);

	/** 获取任务在列表中的索引 */
	bool GetMissionIndexByID(int32 MissionID, int32& OutMissionIndex);
	
protected:
	/** 当前任务列表 */
	UPROPERTY()
	UTaskList* MissionList;

	/** 当前执行的任务 */
	UPROPERTY()
		UMissionObject* Mission;

	/** 执行过程的List */
	UPROPERTY()
		TArray<UTaskObject*> TaskExecuteList;

	/** 当前执行到的任务在列表中的索引 */
	UPROPERTY(BlueprintReadOnly, Category = "MissionManager")
		int32 CurrentMissionIndex;
	/** 当前执行到的任务中的Task在MissionObject列表中的索引 */
	UPROPERTY(BlueprintReadOnly, Category = "MissionManager")
		int32 CurrentTaskIndex;

	/** 当前执行的Task */
	UPROPERTY(BlueprintReadOnly, Category = "MissionManager")
		UTaskObject* CurTask;
	/** 当前执行的Mission */
	UPROPERTY(BlueprintReadOnly, Category = "MissionManager")
		UMissionObject* CurMission;

	/** 是否所有任务执行完毕 */
	UPROPERTY(BlueprintReadOnly, Category = "MissionManager")
		bool bAllMissionComplete;

	UPROPERTY()
		APawn* PlayerPawn;

	/** 持续性Tick的Task列表 */
	UPROPERTY()
		TArray<UTaskObject*> PersistentTickTasks;

	
private:
	/** 缓存上一次的大任务Index，用于监控大任务改变 */
	int32 OldMissionIndex;

	/** 用来标记PersistentTickTasks正在做循环；不能做该数组的添加移除操作 */
	bool bPersistentTickTasksLooping;
	/** 用来缓存将要从PersistentTickTasks移除的Task在该数组中的Index */
	TArray<int32> PersistentTickTaskRemoveIndices;
	/** 用来缓存将要从PersistentTickTasks添加的Task */
	TArray<UTaskObject*> PersistentTickTasksToAdd;

};

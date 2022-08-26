

#pragma once

#include "CoreMinimal.h"
#include "../MissionObject.h"
#include "../TaskObject.h"
#include "Templates/SubclassOf.h"
#include "../MissionInterface.h"
#include "Task_SubMission.generated.h"

/**
 * 
 */
UCLASS()
class MISSIONBUILDER_API UTask_SubMission : public UTaskObject, public IMissionInterface
{
	GENERATED_BODY()

public:
	UTask_SubMission();

	virtual void ExecuteTask(UObject* MissionManagerObj) override;
	/** 中止执行 */
	virtual void AbortTask() override;
	/** 重置执行 */
	virtual void ResetTask() override;
	/** 清空任务，做删除前的清理操作 */
	virtual void ClearTask() override;
	/** 跳过执行，一帧内执行 */
	virtual void SkipTask(UObject* MissionManagerObj) override;
	/** 结束执行 */
	//virtual void FinishExecute(bool bSuccess = true) override;
	/** 执行时的Tick */
	virtual void Tick(float DeltaTime) override;

	/************ Begin IMissionInterafce  ***********/
	virtual void ToNextTask() override;

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
	/** 是否有PersistentTick */
	virtual bool GetHasPersistenTickTask() override;

	/************ End IMissionInterafce  ***********/

	/** 直接设置一个MissionObject(再调用ExecuteTask, 不需要MissionObjectAsset) */
	void SetMissionObject(UMissionObject* MissionObj);

protected:

	void DestroyCurrentMission();

	UFUNCTION()
	void OnSubMissionComplete();
	UFUNCTION()
	void OnTaskComplete(bool Success); 

	void ExecuteTask();

public:
	/** 子任务资源 */
	UPROPERTY(EditAnywhere, Category = "Task", meta = (DisplayName = "子任务资源"))
		UMissionObject* MissionObjectAsset;

protected:

	UPROPERTY()
		class UMissionObject* MissionObject;

	/** 执行过程的List */
	UPROPERTY()
		TArray<UTaskObject*> TaskExecuteList;

	/** 缓存当前Task */
	UPROPERTY()
		UTaskObject* CurrentTask;

	/** 持续性Tick的Task列表 */
	UPROPERTY()
	TArray<UTaskObject*> PersistentTickTasks;

	int32 CurrentTaskIndex;

private:
	/** 用来标记PersistentTickTasks正在做循环；不能做该数组的添加移除操作 */
	bool bPersistentTickTasksLooping;
	/** 用来缓存将要从PersistentTickTasks移除的Task在该数组中的Index */
	TArray<int32> PersistentTickTaskRemoveIndices;
	/** 用来缓存将要从PersistentTickTasks添加的Task */
	TArray<UTaskObject*> PersistentTickTasksToAdd;
	
};

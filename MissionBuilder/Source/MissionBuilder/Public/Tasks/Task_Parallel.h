

#pragma once

#include "CoreMinimal.h"
#include "../TaskObject.h"
#include "../MissionInterface.h"
#include "Task_Parallel.generated.h"

USTRUCT(BlueprintType)
struct FTaskArray
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
		TArray<UTaskObject*> Tasks;
};

class UBranchCallbackBase;
/**
 * 
 */
UCLASS()
class MISSIONBUILDER_API UTask_Parallel : public UTaskObject, public IMissionInterface
{
	GENERATED_BODY()

public:

	UTask_Parallel();

	/************ TaskObject interface ************/
	virtual void ExecuteTask(UObject* MissionManagerObj) override;
	/** 中止执行 */
	virtual void AbortTask() override;
	/** 重置执行 */
	virtual void ResetTask() override;
	/** 清空任务，做删除前的清理操作 */
	virtual void ClearTask() override;
	/** 跳过执行，一帧内执行 */
	virtual void SkipTask(UObject* MissionManagerObj) override;
	/** 执行时的Tick */
	virtual void Tick(float DeltaTime) override;
	/************ TaskObject interface /************


	/************ Begin IMissionInterafce  ***********/

	/** 子任务完成回调 */
	//UFUNCTION(BlueprintCallable, Category = "MissionManager")
	virtual void OnTaskFinished(UTaskObject* Task, bool bSuccess) override;

	/** 注册持续性Tick；任务开始执行后就算FinishTask后还会Tick - 除非调用UnRegister */
	virtual void RegisterPersistentTick(UTaskObject* Task) override;
	/** 取消持续性Tick */
	virtual void UnRegisterPersistentTick(UTaskObject* Task) override;
	/** 是否有PersistentTick */
	virtual bool GetHasPersistenTickTask() override;

	/************ End IMissionInterafce  ***********/


public:
	/** 该节点配置的任务列表数组 */
	UPROPERTY()
	TArray<FTaskArray> TaskLists;

	UPROPERTY()
		TArray<UTaskObject*> RootTasks;

protected:
	/** 持续性Tick的Task列表 */
	UPROPERTY()
		TArray<UTaskObject*> PersistentTickTasks;

	/** 缓存要执行的子任务 */
	UPROPERTY(BlueprintReadOnly)
		TArray<class UTask_SubMission*> SubMissions;
	/** 缓存当前已完成的Task */
	UPROPERTY(BlueprintReadOnly)
		TArray<UTaskObject*> FinishedSubMissions;
private:
	/** 用来标记PersistentTickTasks正在做循环；不能做该数组的添加移除操作 */
	bool bPersistentTickTasksLooping;
	/** 用来缓存将要从PersistentTickTasks移除的Task在该数组中的Index */
	TArray<int32> PersistentTickTaskRemoveIndices;
	/** 用来缓存将要从PersistentTickTasks添加的Task */
	TArray<UTaskObject*> PersistentTickTasksToAdd;
};

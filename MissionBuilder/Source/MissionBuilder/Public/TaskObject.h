

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
//#include "../Defines/CommonDefines.h"
#include "MissionInterface.h"
#include "TaskObject.generated.h"

/** log到屏幕和控制台 */
#define TASKLOG(Verbosity, Color, Format, ...)\
FString __LogStr_ = FString::Printf(TEXT(##Format), ##__VA_ARGS__);	\
UE_LOG(LogMissionManager, Verbosity, TEXT(##Format), ##__VA_ARGS__); \
if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 20.f, Color, __LogStr_);

#if UE_BUILD_SHIPPING
	#ifdef  TASKLOG 
		#undef	TASKLOG
		#define TASKLOG(Verbosity, Color, Format, ...)
	#endif
#endif

DECLARE_LOG_CATEGORY_EXTERN(LogMissionManager, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTaskComplete, bool, Success);

/**
 * 
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class MISSIONBUILDER_API UTaskObject : public UObject
{
	GENERATED_BODY()

public:

	UTaskObject();

	/** 执行 */
	UFUNCTION(BlueprintCallable, Category = "Task")
		virtual void ExecuteTask(UObject* MissionManagerObj);
	/** 中止执行 */
	UFUNCTION(BlueprintCallable, Category = "Task")
		virtual void AbortTask();
	/** 重置执行*/
	UFUNCTION(BlueprintCallable, Category = "Task")
		virtual void ResetTask();
	/** 清空任务，做删除前的清理操作 */
	UFUNCTION(BlueprintCallable, Category = "Task")
		virtual void ClearTask();
	/** 跳过执行，一帧内执行 */
	UFUNCTION(BlueprintCallable, Category = "Task")
		virtual void SkipTask(UObject* MissionManagerObj);
	/** 是否开启任务，如果返回false，任务将忽略执行 */
	UFUNCTION(BlueprintCallable, Category = "Task")
		virtual bool IsEnable() const;
	/** 结束执行 */
	UFUNCTION(BlueprintCallable, Category = "Task")
		virtual void FinishExecute(bool bSuccess = true);
	/** 执行时的Tick */
	UFUNCTION(BlueprintCallable, Category = "Task")
		virtual void Tick(float DeltaTime);

	/** 获取下一个执行的Task */
	virtual UTaskObject* GetNextTask();

#if WITH_ENGINE
	virtual class UWorld* GetWorld() const override;
#endif

protected:
	/** 执行 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Task")
		void OnExecuteTask();
	/** 中止执行 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Task")
		void OnAbortTask();
	/** 重置执行
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Task")
		void OnResetTask();
	/** 清空任务，做删除前的清理操作 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Task")
		void OnClearTask();
	/** 是否开启任务，如果返回false，任务将忽略执行 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Task")
		bool OnIsEnable() const;
	/** 跳过执行，一帧内执行(快速执行） */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Task")
		void OnSkipTask();
	/** 执行时的Tick */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Task")
		void OnTick(float DeltaTime);

	/** 开始持续性Tick */
	UFUNCTION(BlueprintCallable, Category = "Task|Tick")
		void StartPersistentTick();
	/** 停止持续性Tick */
	UFUNCTION(BlueprintCallable, Category = "Task|Tick")
		void StopPersistentTick();

protected:
	/** 根据ActorTag寻找Actor实例 */
	UFUNCTION(BlueprintCallable, Category = "Task|Utils")
		AActor* FindActorByTag(FName ActorTag);
	/** 根据ActorTag寻找所有Actor实例 */
	UFUNCTION(BlueprintCallable, Category = "Task|Utils")
		TArray<AActor*> FindActorsByTag(FName ActorTag);

	/** 根据ComponentTag在Actor中寻找组件实例 - 多个相同Tag则放回第一个 */
	UFUNCTION(BlueprintCallable, Category = "Task|Utils")
		UActorComponent* FindComponentByTag(AActor* InActor, FName ComponentTag);
	/** 根据ComponentTag在Actor中寻找所有组件实例 */
	UFUNCTION(BlueprintCallable, Category = "Task|Utils")
		TArray<UActorComponent*> FindComponentsByTag(AActor* InActor, FName ComponentTag);

	/** 根据ComponentTag在Actor中寻找Scene类型组件实例 - 多个相同Tag则放回第一个 */
	UFUNCTION(BlueprintCallable, Category = "Task|Utils")
		USceneComponent* FindSceneComponentByTag(AActor* InActor, FName ComponentTag);

public:
	/** Task ID */
	UPROPERTY(BlueprintReadOnly, Category = "Task", meta = (DisplayName = "TaskID"))
		int32 TaskID;

	/** 注释掉，采用IsEnable()函数判断是否开启执行 */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (DisplayName = "是否启用"))
	//	bool bEnable;

	/** Task名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (DisplayName = "Task名称"))
		FText TaskName;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (DisplayName = "允许执行模式", Bitmask, BitmaskEnum = ETrainMode))
	//	int32 AllowMode;

	/** 失败后重新执行 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (DisplayName = "失败后重新执行"))
		bool bReRunOnFalied;

	/** 执行开始后一直Tick，就算结束任务执行后还是会Tick，除非调用StoppersistentTick() */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (DisplayName = "执行开始后一直Tick"))
		bool bPersistentTick;

#if WITH_EDITORONLY_DATA

	/** 说明 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (DisplayName = "开发备注"))
		FString Note;
	/** 节点颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (DisplayName = "节点颜色"))
		FLinearColor NodeColor;

#endif // WITH_EDITORONLY_DATA

	UPROPERTY(BlueprintAssignable)
		FOnTaskComplete OnTaskComplete;

	/** 上一个执行Task - 运行时数据，用于反向执行 */
	UPROPERTY(BlueprintReadOnly)
		UTaskObject* LastExecutedTask;
	/** 要执行的下一个Task */
	UPROPERTY(BlueprintReadOnly)
		UTaskObject* NextTask;

protected:
	/** 执行任务前玩家变换 */
	UPROPERTY(BlueprintReadOnly)
		FTransform PlayerTransformCache;

protected:
	//UPROPERTY()
		IMissionInterface* MissionManagerPtr;
	
};

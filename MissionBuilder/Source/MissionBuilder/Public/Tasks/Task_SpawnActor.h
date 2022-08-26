

#pragma once

#include "CoreMinimal.h"
#include "../TaskObject.h"
#include "Task_SpawnActor.generated.h"

class UBranchCallbackBase;
/**
 * 
 */
UCLASS()
class MISSIONBUILDER_API UTask_SpawnActor : public UTaskObject
{
	GENERATED_BODY()

public:
	UTask_SpawnActor();

	virtual void ExecuteTask(UObject* MissionManagerObj) override;

	virtual void ResetTask() override;

	/** 清空任务，做删除前的清理操作 */
	virtual void ClearTask() override;

	virtual void SkipTask(UObject* MissionManagerObj) override;

protected:
	
	void SpawnActor();

public:
	/** 需要生成的Actor类 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (DisplayName = "Actor类"))
		TSubclassOf<AActor> ActorClass;

	/** 生成的Actor赋予的Tag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
		FName ActorTag;

	/** 生成的世界变换 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (DisplayName = "世界变换", EditCondition="!bRelativeTransform"))
		FTransform ActorTransform;

	/** 使用相对变换 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (DisplayName = "是否使用相对变换"))
		bool bRelativeTransform = false;
	/** 相对变换与该Tag的Actor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (DisplayName = "相对变换的ActorTag", EditCondition = "bRelativeTransform"))
		FName RelativeActorTag;
	/** 相对变换的Actor的组件Tag，如果为None则忽略此Tag，结果时相对与Actor的根组件 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (DisplayName = "相对变换的ComponentTag", EditCondition = "bRelativeTransform"))
		FName RelativeComponentTag;
	/** 相对变换 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (DisplayName = "相对变换", EditCondition = "bRelativeTransform"))
		FTransform RelativeTransform;

	
protected:
	/** 缓存生成的Actor */
	UPROPERTY(BlueprintReadOnly)
		AActor* SpawnedActor;
};

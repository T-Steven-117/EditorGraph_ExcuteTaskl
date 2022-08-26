

#pragma once

#include "CoreMinimal.h"
#include "../TaskObject.h"
#include "Task_SpawnActorAttach.generated.h"


/**
 * 
 */
UCLASS()
class MISSIONBUILDER_API UTask_SpawnActorAttach : public UTaskObject
{
	GENERATED_BODY()

public:
	UTask_SpawnActorAttach();

	virtual void ExecuteTask(UObject* MissionManagerObj) override;

	virtual void ResetTask() override;

	virtual void SkipTask(UObject* MissionManagerObj) override;

	/** 清空任务，做删除前的清理操作 */
	virtual void ClearTask() override;
protected:
	
	void SpawnActor();

public:
	/** 需要生成的Actor类 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (DisplayName = "Actor类"))
		TSubclassOf<AActor> ActorClass;

	/** 生成的Actor赋予的Tag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
		FName ActorTag;

	/** 要Attach的Actor Tag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
		FName ActorTagToAttach;

	/** 要Attach的Actor的组件 Tag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
		FName ComponentTagToAttach;

	/** 要Attach的Actor的组件的SocketName */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
		FName SocketToAttach;

	/** 生成的相对于AttachComponent的相对变换 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (DisplayName = "相对变换"))
		FTransform RelativeTransform;
	
protected:
	/** 缓存生成的Actor */
	UPROPERTY(BlueprintReadOnly)
		AActor* SpawnedActor;
};

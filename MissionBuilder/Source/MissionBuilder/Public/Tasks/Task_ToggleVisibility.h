

#pragma once

#include "CoreMinimal.h"
#include "../TaskObject.h"
#include "Task_ToggleVisibility.generated.h"


/**
 *	需要实现ITaskVisibility接口，没有实现则直接调用SetActorHiddenInGame等
 */
UCLASS()
class MISSIONBUILDER_API UTask_ToggleVisibility : public UTaskObject
{
	GENERATED_BODY()

public:
	UTask_ToggleVisibility();

	virtual void ExecuteTask(UObject* MissionManagerObj) override;

	virtual void ResetTask() override;

	virtual void SkipTask(UObject* MissionManagerObj) override;

protected:
	
	void ToggleVisibility(bool IsVisible);

public:
	/** 需要显隐的Actor的Tag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
		FName ActorTag;

	/**  需要显隐的组件Tag （为None表示操作Actor） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
		FName ComponentTag;

	/**是否显示 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (DisplayName = "是否显示"))
		bool Visible;

	/** 是否设置显隐的时候也设置碰撞，显示将开启碰撞，隐藏将关闭碰撞 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (DisplayName = "是否设置碰撞"))
		bool IncludeCollision = true;

};

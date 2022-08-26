

#pragma once

#include "CoreMinimal.h"
#include "../TaskObject.h"
#include "Task_Delay.generated.h"


/**
 * 
 */
UCLASS()
class MISSIONBUILDER_API UTask_Delay : public UTaskObject
{
	GENERATED_BODY()

public:

	UTask_Delay();

	virtual void ExecuteTask(UObject* MissionManagerObj) override;

	virtual void AbortTask() override;

protected:
	/** 延迟回调 */
	UFUNCTION()
		void OnDelayEnd();

public:
	/** 延迟时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (DisplayName = "延迟时间"))
		float Delay;
	
protected:
	FTimerHandle TimerHandle;
};

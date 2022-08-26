

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MissionInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UMissionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MISSIONBUILDER_API IMissionInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintCallable, Category = "MissionInterface")
		virtual void ToNextTask();

	UFUNCTION(BlueprintCallable, Category = "MissionInterface")
		virtual void JumpToTask(int32 TaskIDToJump);

	UFUNCTION(BlueprintCallable, Category = "MissionInterface")
		virtual void OnTaskFinished(UTaskObject* Task, bool bSuccess);

	UFUNCTION(BlueprintCallable, Category = "MissionInterface")
		virtual APawn* GetPlayerPawn();

	UFUNCTION(BlueprintCallable, Category = "MissionInterface")
		virtual FString GetMissionName();

	UFUNCTION(BlueprintCallable, Category = "MissionInterface")
		virtual void RegisterPersistentTick(UTaskObject* Task);
	UFUNCTION(BlueprintCallable, Category = "MissionInterface")
		virtual void UnRegisterPersistentTick(UTaskObject* Task);
	/**  «∑Ò”–PersistentTick */
	UFUNCTION(BlueprintCallable, Category = "MissionInterface")
		virtual bool GetHasPersistenTickTask();
};

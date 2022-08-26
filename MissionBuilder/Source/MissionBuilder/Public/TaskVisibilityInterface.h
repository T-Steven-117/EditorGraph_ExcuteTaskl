// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TaskVisibilityInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTaskVisibilityInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MISSIONBUILDER_API ITaskVisibilityInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/** 切换显隐 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "TaskVisibilityInterface")
		void ToggleVisibility(bool Visible);
};

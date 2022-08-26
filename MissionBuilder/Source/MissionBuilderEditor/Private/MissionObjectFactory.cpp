// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionObjectFactory.h"
#include "../../MissionBuilder/Public/MissionObject.h"

UMissionObjectFactory::UMissionObjectFactory()
{
	// Provide the factory with information about how to handle our asset
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UMissionObject::StaticClass();
}

UObject* UMissionObjectFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	// Create and return a new instance of our MyCustomAsset object
	UMissionObject* MissionObj = NewObject<UMissionObject>(InParent, Class, Name, Flags);
	return MissionObj;
}

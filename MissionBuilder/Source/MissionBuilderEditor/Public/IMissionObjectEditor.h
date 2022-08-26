// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"
//#include "IMissionObjectEditor.generated.h"

class UMissionObject;

/**
 * Public interface to Custom Asset Editor
 */
class IMissionObjectEditor : public FAssetEditorToolkit
{
public:
	/** Retrieves the current custom asset. */
	virtual UMissionObject* GetMissionObject() const = 0;

	/** Set the current custom asset. */
	virtual void SetMissionObject(UMissionObject* InMissioObject) = 0;
};

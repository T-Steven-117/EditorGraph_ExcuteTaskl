

#pragma once

#include "CoreMinimal.h"
#include "BranchCallbackBase.generated.h"

/**
 * 
 */
UCLASS()
class MISSIONBUILDER_API UBranchCallbackBase : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	virtual bool Evaluate();

protected:
	UFUNCTION(BlueprintImplementableEvent)
	bool OnEvaluate();
};

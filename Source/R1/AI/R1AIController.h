

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "R1AIController.generated.h"

/**
 * 
 */
UCLASS()
class R1_API AR1AIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AR1AIController(const FObjectInitializer& ObjectInitializer);

	virtual void OnPossess(APawn * InPawn) override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<class UAIPerceptionComponent> AIPerception;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<class UAISenseConfig_Sight> SightConfig;

	// 타겟을 발견하거나 놓쳤을 때 호출될 함수 (이벤트)
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

protected:
	// 블루프린트에서 수정할 수 있도록 FName으로 노출
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite, Category = "Blackboard")
	FName TargetKeyName;
};



#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_PrepareSkill.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UBTService_PrepareSkill : public UBTService_BlackboardBase
{
	GENERATED_BODY()
public:
	UBTService_PrepareSkill();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

public:
	// 거리를 계산할 타겟(플레이어)을 가리키는 블랙보드 키
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector BBKey_TargetActor;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector BBKey_TargetAbilityClass;
};



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

public:
	// 에디터에서 지정할 블랙보드 키
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector BBKey_CanAttack;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector BBKey_TargetAbilityClass;
};

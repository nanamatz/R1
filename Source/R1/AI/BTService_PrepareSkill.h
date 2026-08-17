

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

	// 사거리+각도 판정 결과를 여기에 기록한다 (Bool). BT의 Blackboard 데코레이터가
	// 이 키를 관찰하면 거리 변화만으로도 분기를 선점할 수 있다 — 커스텀 거리
	// 데코레이터는 블랙보드 키가 바뀔 때만 재평가되므로 그것만으로는 불가능하다.
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector BBKey_CanAttack;


	UPROPERTY(EditAnywhere)
	float DistanceMargin = 0.8;
};

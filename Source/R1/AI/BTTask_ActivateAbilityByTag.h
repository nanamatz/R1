#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"
#include "BTTask_ActivateAbilityByTag.generated.h"

/**
 * 몬스터가 지정된 GameplayTag 어빌리티를 활성화하는 BT 태스크 공통 베이스.
 * 서브클래스는 생성자에서 AbilityTag(및 NodeName)를 설정한다.
 */
UCLASS(Abstract)
class R1_API UBTTask_ActivateAbilityByTag : public UBTTaskNode
{
	GENERATED_BODY()

public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

public:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;

protected:
	// 서브클래스 생성자에서 활성화할 어빌리티 태그를 지정한다.
	UPROPERTY()
	FGameplayTag AbilityTag;
};

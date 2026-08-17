

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AbilitySystemComponent.h"
#include "BTTask_ExecuteSkill.generated.h"

struct FBTExecuteSkillMemory
{
	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
	TWeakObjectPtr<UClass> ExecutingClass;
	FDelegateHandle DelegateHandle;
};

/**
 * 
 */
UCLASS()
class R1_API UBTTask_ExecuteSkill : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_ExecuteSkill();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTExecuteSkillMemory); }

public:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector BBKey_TargetAbilityClass;

	// 지정하면 블랙보드 대신 이 클래스를 발동한다. BT에서 특정 스킬을 조건부로 직접
	// 걸고 싶을 때 사용 (예: 거리 decorator 아래에 돌진). 비워두면 기존 블랙보드 경로.
	UPROPERTY(EditAnywhere, Category = "Ability")
	TSubclassOf<class UGameplayAbility> AbilityClassOverride;

private:
	void OnAbilityEnded(class UGameplayAbility* EndedAbility);
};

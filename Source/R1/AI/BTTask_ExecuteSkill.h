

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
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTExecuteSkillMemory); }

public:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector BBKey_TargetAbilityClass;

private:
	void OnAbilityEnded(class UGameplayAbility* EndedAbility);
};

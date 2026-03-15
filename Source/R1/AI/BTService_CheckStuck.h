

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_CheckStuck.generated.h"

/**
 * 
 */
struct FBTCheckStuckMemory
{
	float TimeStuck;
};

UCLASS()
class R1_API UBTService_CheckStuck : public UBTService_BlackboardBase
{
	GENERATED_BODY()
public:
	UBTService_CheckStuck();

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTCheckStuckMemory); }

	// 포기하기까지 걸리는 제한 시간 (에디터에서 수정 가능, 기본 3초)
	UPROPERTY(EditAnywhere, Category = "AI")
	float StuckTimeout = 3.f;
};

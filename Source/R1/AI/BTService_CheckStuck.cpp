


#include "AI/BTService_CheckStuck.h"
#include "R1LogChannels.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/R1AIController.h"
#include "Character/R1RangerMonster.h"
#include "Perception/AIPerceptionComponent.h"

UBTService_CheckStuck::UBTService_CheckStuck()
{
	NodeName = TEXT("Check Stuck (Timeout)");
	bNotifyBecomeRelevant = true;
	bNotifyTick = true;
}

void UBTService_CheckStuck::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	FBTCheckStuckMemory* Memory = reinterpret_cast<FBTCheckStuckMemory*>(NodeMemory);
	if (Memory)
	{
		Memory->TimeStuck = 0.0f;
	}
}

void UBTService_CheckStuck::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	FBTCheckStuckMemory* Memory = reinterpret_cast<FBTCheckStuckMemory*>(NodeMemory);
	if (!Memory) return;

	AAIController* AIC = OwnerComp.GetAIOwner();
	APawn* Pawn = AIC ? AIC->GetPawn() : nullptr;
	if (!Pawn) return;

	// 1. 몬스터가 움직이고 있는지 확인 (속도가 10 이상이면 움직이는 것으로 간주)
	float Speed = Pawn->GetVelocity().Size2D();
	bool bIsMoving = Speed > 10.0f;

	// 2. 몬스터가 공격 중인지 확인 (질문자님이 만드신 상태 머신 활용)
	AR1Character* Monster = Cast<AR1Character>(Pawn);
	bool bIsAttacking = false;
	if (Monster)
	{
		// 어빌리티를 캐스팅 중이거나 공격 몽타주가 재생 중이면 공격하는 것으로 간주
		bIsAttacking = (Monster->GetCreatureState() == ECreatureState::Casting);
	}

	// 🌟 핵심 로직: 몬스터가 자기 할 일(이동 or 공격)을 하고 있다면 타이머를 0으로 리셋!
	if (bIsMoving || bIsAttacking)
	{
		Memory->TimeStuck = 0.0f;
	}
	else
	{
		// 아무것도 안 하고 멍때리고 있다면 타이머 증가! (교착 상태)
		Memory->TimeStuck += DeltaSeconds;

		// 세팅한 시간(예: 3초)을 초과했다면 타겟 포기
		if (Memory->TimeStuck >= StuckTimeout)
		{
			UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
			if (BB)
			{
				AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(GetSelectedBlackboardKey()));
				BB->ClearValue(GetSelectedBlackboardKey());
				
				if (AIC && TargetActor)
				{
					if (UAIPerceptionComponent* Perception = AIC->GetAIPerceptionComponent())
					{
						// 퍼셉션에서 타겟을 잊게 하여, 나중에 다시 감지될 기회를 줍니다.
						// (그냥 ClearValue만 하면 퍼셉션이 업데이트되지 않아 영원히 타겟을 못 찾을 수 있음)
						Perception->ForgetActor(TargetActor);
					}
				}

				UE_LOG(LogR1, Warning, TEXT("몬스터가 %.1f초 동안 아무것도 하지 못해 타겟을 포기합니다!"), StuckTimeout);

				// 포기한 직후 다시 바로 타겟을 잡지 않도록 타이머 초기화
				Memory->TimeStuck = 0.0f;
			}
		}
	}
}

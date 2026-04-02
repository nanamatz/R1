


#include "AI/BTDecorator_HasSight.h"
#include "AI/R1AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/World.h"

UBTDecorator_HasSight::UBTDecorator_HasSight()
{
	NodeName = TEXT("HasSight");
}

bool UBTDecorator_HasSight::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
	AActor* Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(GetSelectedBlackboardKey()));

	if (!ControllingPawn || !Target) return false;

	// 몬스터 눈 위치(보통 머리나 액터 중심)에서 타겟을 향해 레이저(LineTrace)를 쏩니다.
	FVector StartLocation = ControllingPawn->GetActorLocation() + 50.f;
	FVector EndLocation = Target->GetActorLocation();

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(ControllingPawn); // 자기 자신은 무시

	// Visibility 채널을 쏴서 중간에 벽이나 장애물이 있는지 검사합니다.
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_GameTraceChannel2, Params);

	if (bHit)
	{
		// 중간에 무언가 맞았는데, 그게 타겟(플레이어)이라면 시야가 뚫려있는 것!
		if (HitResult.GetActor() == Target)
		{
			return true;
		}
		// 타겟이 아닌 다른 것(벽, 기둥 등)에 맞았다면 시야가 막힌 것!
		return false;
	}

	// 아무것도 안 맞았다면 시야가 뚫려있다고 간주
	return true;
}

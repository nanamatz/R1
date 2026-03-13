


#include "AI/BTTask_MoveAway.h"
#include "AI/R1AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

UBTTask_MoveAway::UBTTask_MoveAway()
{
	NodeName = TEXT("Move Away From Target");
}

EBTNodeResult::Type UBTTask_MoveAway::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AR1AIController* AIC = Cast<AR1AIController>(OwnerComp.GetAIOwner());

	APawn* ControllingPawn = AIC ? AIC->GetPawn() : nullptr;

	AActor* Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(GetSelectedBlackboardKey()));

	if (!ControllingPawn || !Target) return EBTNodeResult::Failed;

	// 1. 타겟으로부터 나를 향하는 '반대 방향' 벡터를 구합니다.
	FVector DirectionAway = (ControllingPawn->GetActorLocation() - Target->GetActorLocation()).GetSafeNormal();

	// 2. 그 방향으로 FleeDistance 만큼 떨어진 목표 지점을 계산합니다.
	FVector FleeLocation = ControllingPawn->GetActorLocation() + (DirectionAway * FleeDistance);

	// 3. 네비게이션 시스템을 이용해 목표 지점 근처의 '이동 가능한' 안전한 좌표를 찾습니다.
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	FNavLocation SafeLocation;
	if (NavSystem && NavSystem->GetRandomPointInNavigableRadius(FleeLocation, 200.0f, SafeLocation))
	{
		// 4. 안전한 곳으로 도망갑니다!
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(AIC, SafeLocation.Location);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed; // 도망갈 곳이 없으면(구석에 몰리면) 실패
}

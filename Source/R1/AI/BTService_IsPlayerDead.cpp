


#include "AI/BTService_IsPlayerDead.h"
#include "Character/R1Player.h"
#include "Character/R1Monster.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/R1AIController.h"

UBTService_IsPlayerDead::UBTService_IsPlayerDead()
{
    NodeName = TEXT("CheckPlayerDeadService");
    Interval = 0.5f;
}

void UBTService_IsPlayerDead::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    // 1. 현재 블랙보드에 저장된 타겟을 가져옴
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp) return;

    // GetSelectedBlackboardKey()는 에디터에서 지정한 키(TargetActor)를 가리킴
    UObject* TargetObject = BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName);

    // 2. 타겟이 유효한지 확인
    AR1Character* TargetCharacter = Cast<AR1Character>(TargetObject);
    if (TargetCharacter)
    {
        // 3. 타겟이 죽었는지 확인 (작성하신 Enum 활용)
        if (TargetCharacter->GetCreatureState() == ECreatureState::Dead)
        {
            // 4. 죽었다면 타겟 정보를 지움 (null로 밀어버리기)
            BlackboardComp->ClearValue(TargetKey.SelectedKeyName);

            // (옵션) 몬스터의 상태를 대기(Idle) 등으로 바꿀 수도 있음
        }
    }
}

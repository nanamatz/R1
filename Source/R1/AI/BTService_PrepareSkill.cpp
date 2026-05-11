


#include "AI/BTService_PrepareSkill.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystem/Abilities/R1GameplayAbility.h"
#include "Character/R1Boss.h"

UBTService_PrepareSkill::UBTService_PrepareSkill()
{
	NodeName = TEXT("Prepare Boss Skill");
	Interval = 0.5f;
	RandomDeviation = 0.1f;
}

void UBTService_PrepareSkill::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) return;

	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC) return;

	AR1Boss* BossCharacter = Cast<AR1Boss>(AIC->GetPawn());
	if (!BossCharacter) return;

	// 1. 공격 가능 여부 (True면 근접, False면 원거리 패턴으로 간주)
	bool bCanAttack = BlackboardComp->GetValueAsBool(BBKey_CanAttack.SelectedKeyName);

	// 2. 이미 선택된 어빌리티가 있다면 스킵 (태스크에서 실행 후 초기화해주길 기대)
	UClass* CurrentTargetClass = BlackboardComp->GetValueAsClass(BBKey_TargetAbilityClass.SelectedKeyName);
	if (CurrentTargetClass) return;

	// 3. 조건에 맞는 어빌리티 배열 가져오기 (AR1Boss의 Getter 사용)
	TArray<TSubclassOf<UGameplayAbility>> AbilityList = bCanAttack ? BossCharacter->GetDefaultSkillList() : BossCharacter->GetAdditionalSkillList();

	// 4. 배열에서 랜덤으로 하나 뽑아 블랙보드에 저장
	if (AbilityList.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, AbilityList.Num() - 1);
		UClass* SelectedClass = AbilityList[RandomIndex];

		// Class 타입으로 블랙보드에 저장합니다.
		BlackboardComp->SetValueAsClass(BBKey_TargetAbilityClass.SelectedKeyName, SelectedClass);
	}
}

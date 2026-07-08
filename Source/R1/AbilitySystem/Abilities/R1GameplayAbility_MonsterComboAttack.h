
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/R1GameplayAbility_MonsterMeeleAttack.h"
#include "R1GameplayAbility_MonsterComboAttack.generated.h"

/**
 * 몬스터 콤보 공격 — 콤보 순환/섹터 데미지는 부모(MonsterMeeleAttack → Attack)가 처리하고,
 * 이 클래스는 기본값(SkillID, ComboSections)만 다르게 설정한다.
 */
UCLASS()
class R1_API UR1GameplayAbility_MonsterComboAttack : public UR1GameplayAbility_MonsterMeeleAttack
{
	GENERATED_BODY()

public:
	UR1GameplayAbility_MonsterComboAttack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};

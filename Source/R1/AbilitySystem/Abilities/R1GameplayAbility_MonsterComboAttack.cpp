
#include "AbilitySystem/Abilities/R1GameplayAbility_MonsterComboAttack.h"

UR1GameplayAbility_MonsterComboAttack::UR1GameplayAbility_MonsterComboAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SkillID = FName("MonsterComboAttack");
	ComboSections = { FName("Combo1"), FName("Combo2") };
}

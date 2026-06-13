#include "AI/BTTask_SpecialAttack.h"
#include "R1GameplayTags.h"

UBTTask_SpecialAttack::UBTTask_SpecialAttack()
{
	NodeName = TEXT("SpecialAttack");
	AbilityTag = R1GameplayTags::Ability_Monster_SpecialAttack;
}

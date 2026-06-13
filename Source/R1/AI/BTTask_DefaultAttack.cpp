#include "AI/BTTask_DefaultAttack.h"
#include "R1GameplayTags.h"

UBTTask_DefaultAttack::UBTTask_DefaultAttack()
{
	NodeName = TEXT("DefaultAttack");
	AbilityTag = R1GameplayTags::Ability_Monster_Attack;
}

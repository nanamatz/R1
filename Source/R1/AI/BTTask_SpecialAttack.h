#pragma once

#include "CoreMinimal.h"
#include "AI/BTTask_ActivateAbilityByTag.h"
#include "BTTask_SpecialAttack.generated.h"

/**
 * 몬스터의 특수 공격 어빌리티를 활성화하는 BT 태스크.
 */
UCLASS()
class R1_API UBTTask_SpecialAttack : public UBTTask_ActivateAbilityByTag
{
	GENERATED_BODY()

public:
	UBTTask_SpecialAttack();
};

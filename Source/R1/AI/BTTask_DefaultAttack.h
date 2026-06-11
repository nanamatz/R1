#pragma once

#include "CoreMinimal.h"
#include "AI/BTTask_ActivateAbilityByTag.h"
#include "BTTask_DefaultAttack.generated.h"

/**
 * 몬스터의 기본 공격 어빌리티를 활성화하는 BT 태스크.
 */
UCLASS()
class R1_API UBTTask_DefaultAttack : public UBTTask_ActivateAbilityByTag
{
	GENERATED_BODY()

public:
	UBTTask_DefaultAttack();
};

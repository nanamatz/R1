#pragma once

#include "CoreMinimal.h"
#include "Character/R1Boss.h"
#include "BossHierarch.generated.h"

/**
 * 5F 최종 보스 — 소환형(Summoner).
 * 스킬 목록/페이즈/투사체 설정은 전부 BP_Hierarch에 있다. 이 클래스는 타입 구분용.
 */
UCLASS()
class R1_API ABossHierarch : public AR1Boss
{
	GENERATED_BODY()
};

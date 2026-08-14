#pragma once

#include "CoreMinimal.h"
#include "Character/R1Boss.h"
#include "BossWarden.generated.h"

/**
 * 2F 보스 — 거리 유지형(Zoner).
 * 스킬 목록/페이즈/투사체 설정은 전부 BP_Warden에 있다. 이 클래스는 타입 구분용.
 */
UCLASS()
class R1_API ABossWarden : public AR1Boss
{
	GENERATED_BODY()
};

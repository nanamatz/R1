#pragma once

#include "CoreMinimal.h"
#include "Character/R1Boss.h"
#include "BossRavager.generated.h"

/**
 * 3F 보스 — 근접 압박형(Bruiser).
 * 스킬 목록/페이즈 설정은 전부 BP_Ravager에 있다. 이 클래스는 타입 구분용.
 */
UCLASS()
class R1_API ABossRavager : public AR1Boss
{
	GENERATED_BODY()
};

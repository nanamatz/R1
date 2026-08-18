

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "R1AbilitySystemLibrary.generated.h"

class AR1Monster;
/**
 * 
 */
UCLASS()
class R1_API UR1AbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION( Category = "Targeting")
	static TArray<AR1Character*> GetChainLightningTargets(AR1Character* SourceActor, AR1Character* InitialTarget, float BounceRadius, int32 MaxBounces);

	// 몬스터 → 플레이어 부채꼴(섹터) 근접 판정 공통 로직.
	// 소스 ASC의 AttackRange/AttackRadius 어트리뷰트로 구 오버랩 + 내적 각도 검증 후,
	// 판정에 든 모든 플레이어에게 SpecHandle의 GameplayEffect를 적용한다.
	// OutDamagedPlayers를 넘기면 실제 피해를 입힌 플레이어 목록을 채워준다 (히트 이펙트 등 후처리용).
	static void ApplySectorDamageToPlayers(const struct FGameplayEffectSpecHandle& SpecHandle, AR1Character* SourceCharacter, class UAbilitySystemComponent* SourceASC, TArray<AActor*>* OutDamagedPlayers = nullptr);

	// 메타 업그레이드로만 오르는 플레이어 전역 보너스 스탯(ExtraGold/ExtraExp/Luck 등)을 월드 어디서든 읽는다.
	// 값은 퍼센트 단위(10 = +10%). 플레이어나 ASC가 없으면 0을 돌려준다.
	static float GetPlayerMetaBonus(const UObject* WorldContextObject, const struct FGameplayAttribute& Attribute);
};

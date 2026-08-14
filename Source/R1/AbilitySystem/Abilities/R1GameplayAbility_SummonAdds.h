#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/R1GameplayAbility_BossAttackBase.h"
#include "R1GameplayAbility_SummonAdds.generated.h"

class AR1Monster;

/**
 * 보스 소환 어빌리티. 아바타 주변 링 위에 잡몹을 스폰한다.
 * AR1MonsterSpawner는 레벨에 배치된 액터(ADungeonManager 종속)라 어빌리티에서 재사용할 수 없어 별도 구현.
 */
UCLASS()
class R1_API UR1GameplayAbility_SummonAdds : public UR1GameplayAbility_BossAttackBase
{
	GENERATED_BODY()

public:
	// 베이스(BossAttackBase)가 FObjectInitializer 생성자를 노출하지 않으므로 기본 생성자로 맞춘다.
	UR1GameplayAbility_SummonAdds();

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void OnAttackEventReceived(FGameplayEventData Payload) override;

private:
	// SpawnedAdds에서 죽었거나 사라진 항목을 제거하고 남은 수를 반환한다.
	int32 PruneAndCountAlive();

protected:
	// 스폰 후보. 매 스폰마다 하나를 무작위 선택한다.
	UPROPERTY(EditAnywhere, Category = "Summon")
	TArray<TSubclassOf<AR1Monster>> AddClasses;

	UPROPERTY(EditAnywhere, Category = "Summon", meta = (ClampMin = "1"))
	int32 SpawnCount = 2;

	// 살아있는 소환수가 이 수 이상이면 발동 자체를 거부한다.
	UPROPERTY(EditAnywhere, Category = "Summon", meta = (ClampMin = "1"))
	int32 AliveCap = 4;

	UPROPERTY(EditAnywhere, Category = "Summon", meta = (ClampMin = "0.0"))
	float SpawnRingRadius = 400.0f;

private:
	// 이 어빌리티는 InstancedPerActor이므로 액티베이션 사이에 이 배열이 유지된다.
	// 그래서 살아있는 소환수 추적이 성립한다.
	// UHT는 UPROPERTY에 mutable을 허용하지 않으므로, const 함수에서는 const_cast로 접근한다.
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AR1Monster>> SpawnedAdds;
};



#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "R1GameplayAbility.generated.h"

UENUM(BlueprintType)
enum class ER1SkillType : uint8
{
	Active,
	Passive
};

UCLASS()
class R1_API UR1GameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UR1GameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	ER1SkillType GetSkillType() const { return SkillType; }

protected:
	// 🌟 공격류 어빌리티 공통 보일러플레이트 (Template Method):
	// Casting 전환 → AttackSpeed 배속으로 MontageToPlay 재생(종료/중단/취소 시 OnMontageEnded)
	// → InAttackEventTag 게임플레이 이벤트 대기(수신 시 OnAttackEventReceived).
	// Attacker 또는 MontageToPlay가 없으면 아무것도 하지 않고 false 반환.
	// 몽타주 시작이 동기 실패해 어빌리티가 이미 종료된 경우에도 false 반환 —
	// 호출부는 IsActive()를 확인한 뒤에만 EndAbility를 호출할 것 (이중 종료 방지).
	// bTriggerEventOnce=false로 주면 한 번의 액티베이션에서 노티파이가 여러 번 와도 매번 처리한다
	// (한 섹션에 타격 애님이 2개 이상인 콤보용). 기본값 true는 기존 동작 그대로.
	bool PlayAttackMontageAndWaitForEvent(class AR1Character* Attacker, const FGameplayTag& InAttackEventTag, FName MontageStartSection = NAME_None, bool bTriggerEventOnce = true);

	// 위 보일러플레이트가 바인딩하는 핸들러. 서브클래스에서 override (UFUNCTION 재선언 금지 — UHT 중복 에러).
	UFUNCTION()
	virtual void OnMontageEnded();

	UFUNCTION()
	virtual void OnAttackEventReceived(FGameplayEventData Payload);

protected:
	UPROPERTY(EditAnywhere, Category = "Skill")
	ER1SkillType SkillType = ER1SkillType::Passive;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<class UAnimMontage> MontageToPlay;

	UPROPERTY(EditAnywhere, Category = "Audio")
	TObjectPtr<class USoundBase> SoundToPlay;

	// 명중 시 피격 지점에 재생할 어빌리티 전용 임팩트 이펙트 (Niagara).
	// 설정 시 GameplayCue.Weapon.Impact 큐에 SourceObject로 전달되어 무기 DA/기본 VFX보다 우선한다.
	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	TObjectPtr<class UNiagaraSystem> HitImpactVFX;

	// 레거시 Cascade 임팩트 이펙트. HitImpactVFX(Niagara)와 둘 다 지정되면 Niagara가 우선한다.
	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	TObjectPtr<class UParticleSystem> HitImpactCascade;

	// 큐 SourceObject로 전달할 임팩트 이펙트 반환 (Niagara 우선, 없으면 Cascade, 둘 다 없으면 nullptr)
	UObject* GetHitImpactEffect() const;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tooltip")
	FName SkillNameKey;

	UAnimMontage* GetMontageToPlay() const { return MontageToPlay; }
	USoundBase* GetSoundToPlay() const { return SoundToPlay; }
};

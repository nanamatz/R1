

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
	bool PlayAttackMontageAndWaitForEvent(class AR1Character* Attacker, const FGameplayTag& InAttackEventTag, FName MontageStartSection = NAME_None);

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

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tooltip")
	FName SkillNameKey;

	UAnimMontage* GetMontageToPlay() const { return MontageToPlay; }
	USoundBase* GetSoundToPlay() const { return SoundToPlay; }
};



#pragma once

#include "CoreMinimal.h"
#include "Character/R1Character.h"
#include "R1Monster.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMonsterReadyToSleep, class AR1Monster*, DeadMonster);

UCLASS()
class R1_API AR1Monster : public AR1Character
{
	GENERATED_BODY()

public:
	AR1Monster();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void InitAbilitySystem() override;

	//void DefaultAttack();
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TObjectPtr<class UWidgetComponent> HpBarComponent;

	UFUNCTION()
	void RefreshHpBar(float Ratio);

	class UR1AttributeSet* GetR1AttributeSet() const { return CoreAttributeSet; }

public:
	void ActivateAbility(FGameplayTag AbilityTag);
public:
	virtual void OnDead(const TObjectPtr<class AR1Character> Attacker) override;

public:
	// 스포너가 태어날 때 호출해 줄 주입 함수
	void InitializeWithManager(class ADungeonManager* InManager);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<class UMonsterAttributeSet> MonsterAttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<class UR1AttributeSet> CoreAttributeSet;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	TObjectPtr<class UAnimMontage> DeathAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TSubclassOf<class UGameplayEffect> XpEffect;

	float AggroRange;

protected:
	virtual void InitAttributes() override;

	// 몬스터 전용 초기화 GE (GE_InitMonsterStats 할당)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Init")
	TSubclassOf<class UGameplayEffect> MonsterInitStatEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material")
	TObjectPtr<class UMaterialInstanceDynamic> DissolveMaterial;

	struct FTimerHandle DissolveTimerHandle;

	float CurrentDissolve = 0.0f;

	// [추가] 5초 대기용 타이머 핸들
	struct FTimerHandle DissolveDelayTimerHandle;

	// [추가] 5초 뒤에 실제로 디졸브를 시작할 함수
	void StartDissolve();

	void UpdateDissolve();

public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMonsterReadyToSleep OnReadyToSleep;

	UFUNCTION(BlueprintCallable, Category = "Pool")
	virtual void WakeUp();

	UFUNCTION(BlueprintCallable, Category = "Pool")
	virtual void GoToSleep();
};

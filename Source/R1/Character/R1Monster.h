

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

	UAnimMontage* GetHitReactMontage() const { return HitReactMontage; }

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<class UAnimMontage> HitReactMontage;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TSubclassOf<class UGameplayEffect> XpEffect;

protected:
	virtual void InitAttributes() override;

	// 몬스터 전용 초기화 GE (GE_InitMonsterStats 할당)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Init")
	TSubclassOf<class UGameplayEffect> MonsterInitStatEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material")
	TObjectPtr<class UMaterialInstanceDynamic> DissolveMaterial;

	struct FTimerHandle DissolveTimerHandle;

	float CurrentDissolve = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	float DissolveConstant = 0.03f;

	struct FTimerHandle DissolveDelayTimerHandle;

	void StartDissolve();

	void UpdateDissolve();

public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMonsterReadyToSleep OnReadyToSleep;

	UFUNCTION(BlueprintCallable, Category = "Pool")
	virtual void WakeUp();

	UFUNCTION(BlueprintCallable, Category = "Pool")
	virtual void GoToSleep();

protected:
	// 에디터에서 이 몬스터가 사용할 비헤이비어 트리를 할당할 수 있게 뚫어줍니다.
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<class UBehaviorTree> DefaultBehaviorTree;

public:
	// 컨트롤러가 가져갈 수 있게 Getter 함수 제공
	class UBehaviorTree* GetBehaviorTree() const { return DefaultBehaviorTree; }
};

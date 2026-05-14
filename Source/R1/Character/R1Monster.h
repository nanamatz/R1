

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

	virtual void Highlight() override;
	virtual void UnHighlight() override;

public:

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<class UAnimMontage> DeathAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	TSubclassOf<class UGameplayEffect> XpEffect;

protected:
	virtual void OnHealthChanged(float Ratio, bool bIsDamage) override;
	virtual void InitAttributes() override;

	// 몬스터 전용 초기화 GE (GE_InitMonsterStats 할당)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<class UGameplayEffect> MonsterInitStatEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material")
	TArray<TObjectPtr<class UMaterialInstanceDynamic>> DissolveMaterials;

	struct FTimerHandle DissolveTimerHandle;

	float CurrentDissolve = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	float DissolveConstant = 0.03f;

	struct FTimerHandle DissolveDelayTimerHandle;

	void StartDissolve();

	void UpdateDissolve();

protected:
	// 🌟 언리얼 에디터에서 방금 만든 BP_GoldActor를 할당할 수 있는 변수 추가!
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	TSubclassOf<class AR1GoldActor> GoldActorClass;

	UFUNCTION(BlueprintCallable, Category = "Reward")
	virtual void DropGold();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	float GoldDropChance = 0.7f; // 70% 확률로 골드 드랍

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 MinGoldDrop = 15; // 최소 골드 드랍량

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 MaxGoldDrop = 400; // 최대 골드 드랍량

	virtual void RewardExperience(const TObjectPtr<class AR1Character> Attacker);
public:
	UPROPERTY(BlueprintAssignable, Category = "Pool")
	FOnMonsterReadyToSleep OnReadyToSleep;

	UFUNCTION(BlueprintCallable, Category = "Pool")
	virtual void WakeUp();

	UFUNCTION(BlueprintCallable, Category = "Pool")
	virtual void GoToSleep();

protected:
	// 에디터에서 이 몬스터가 사용할 비헤이비어 트리를 할당할 수 있게 뚫어줍니다.
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<class UBehaviorTree> DefaultBehaviorTree;

	// 몬스터가 죽을 때 나는 소리
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<class USoundBase> DeathSound;

public:
	// 컨트롤러가 가져갈 수 있게 Getter 함수 제공
	class UBehaviorTree* GetBehaviorTree() const { return DefaultBehaviorTree; }

protected:
	// 블루프린트에서 방금 만든 M_HitFlashOverlay 머티리얼을 할당받을 변수
	UPROPERTY(EditAnywhere, Category = "Material")
	TObjectPtr<UMaterialInterface> HitFlashOverlayMaterial;

	FTimerHandle HitFlashTimerHandle;
	void ResetHitFlash();
};

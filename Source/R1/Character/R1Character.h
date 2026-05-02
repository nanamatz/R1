

#pragma once

#include "AbilitySystemInterface.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "Interface/R1HighlightInterface.h"
#include "R1Define.h"
#include "R1Character.generated.h"

class AR1Character;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHpChangedDelegate,float,Ratio);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeadDelegate, AR1Character*, DeadCharacter, AR1Character*, Attacker);

UCLASS()
class R1_API AR1Character : public ACharacter, public  IR1HighlightInterface, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AR1Character();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void HandleGameplayEvent(FGameplayTag EventTag);

public:
	virtual void Highlight() override;
	virtual void UnHighlight() override;

	virtual void OnDamaged(int32 Damage, TObjectPtr<AR1Character> Attacker);
	virtual void OnDead(const TObjectPtr<AR1Character> Attacker);

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void InitAbilitySystem();

public:
	UPROPERTY(BlueprintAssignable, Category = "Attributes")
	FOnHpChangedDelegate OnHpChanged;

	UPROPERTY(BlueprintAssignable, Category = "State")
	FOnDeadDelegate OnDeadDelegate;

	virtual void OnHealthChanged(float Ratio);
	float GetHealthRatio() const;

public:
	// 변수를 부모로 이동
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	ECreatureState CreatureState = ECreatureState::Idle;

	// 공통 함수는 부모에서 한 번만 구현
	void SetCreatureState(ECreatureState InState);
	ECreatureState GetCreatureState() const { return CreatureState; }
	FName GetCharacterRowName() const { return CharacterRowName; }
	bool GetIsWeaponEquipped() const { return bIsWeaponEquipped; }

protected:
	UFUNCTION()
	virtual void OnWeaponChanged(bool bIsEquipped);

protected:
	// 1. 데이터 테이블 에셋 (에디터에서 지정)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Init")
	TObjectPtr<class UDataTable> CharacterStatTable;

	// 2. 초기화용 GE 클래스 (GE_InitStats 넣을 곳)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Init")
	TSubclassOf<class UGameplayEffect> InitStatEffectClass;

	// 3. 내 이름 (Row Name) - Player는 "Player", 몬스터는 "Goblin" 등
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Init")
	FName CharacterRowName;

protected:
	UPROPERTY(BlueprintReadOnly)
	bool bHighlighted = false;

private:
	bool bIsWeaponEquipped = false;

protected:
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	TObjectPtr<class UR1AbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UR1AttributeSet> CommonAttributeSet;

public:
	//초기화 함수
	void AddCharacterAbility();	
	void ApplyCharacterEffect();
	virtual void InitAttributes();
public:

	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<class UGameplayAbility>> StartupAbilities;

	UPROPERTY(EditAnywhere, Category = "Effects")
	TArray<TSubclassOf<class UGameplayEffect>> StartupEffects;

	UPROPERTY(EditAnywhere, Category="Animation")
	TObjectPtr<class UAnimMontage> AttackMontage;

};
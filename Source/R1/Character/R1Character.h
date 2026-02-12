

#pragma once

#include "AbilitySystemInterface.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "Interface/R1HighlightInterface.h"
#include "R1Define.h"
#include "R1Character.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHpChangedDelegate,float,Ratio);

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

	virtual void OnHealthChanged(float Ratio);
public:
	// 변수를 부모로 이동
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	ECreatureState CreatureState = ECreatureState::None;

	// 공통 함수는 부모에서 한 번만 구현
	void SetCreatureState(ECreatureState InState);
	ECreatureState GetCreatureState() const { return CreatureState; }

protected:
	UPROPERTY(BlueprintReadOnly)
	bool bHighlighted = false;

protected:
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	TObjectPtr<class UR1AbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UR1AttributeSet> AttributeSet;

public:
	void AddCharacterAbility();	
	void InitializeCharacterAttribute();
	void ApplyCharacterEffect();

public:

	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<class UGameplayAbility>> StartupAbilities;

	UPROPERTY(EditAnywhere, Category = "Effects")
	TArray<TSubclassOf<class UGameplayEffect>> StartupEffects;

	UPROPERTY(EditAnywhere, Category="Animation")
	TObjectPtr<class UAnimMontage> AttackMontage;

};



#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/R1HighlightInterface.h"
#include "R1Define.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemInterface.h"
#include "R1Character.generated.h"


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
	virtual void OnDead(TObjectPtr<AR1Character> Attacker);

	void RefreshHpBarRatio();
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void InitAbilitySystem();

protected:
	UPROPERTY(BlueprintReadOnly)
	bool bHighlighted = false;

public:
	UPROPERTY(BlueprintReadWrite)
	ECreatureState CreatureState = ECreatureState::Moving;

public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TObjectPtr<class UWidgetComponent> HpBarComponent;

protected:
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	TObjectPtr<class UR1AbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UR1AttributeSet> AttributeSet;

public:
	void AddCharacterAbility();

	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<class UGameplayAbility>> StartupAbilities;
};

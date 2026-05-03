

#pragma once

#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "R1AttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class R1_API UR1AttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UR1AttributeSet();

	// Common
	ATTRIBUTE_ACCESSORS(ThisClass, Health);
	ATTRIBUTE_ACCESSORS(ThisClass, MaxHealth);
	ATTRIBUTE_ACCESSORS(ThisClass, HealthRegeneration);
	ATTRIBUTE_ACCESSORS(ThisClass, MoveSpeed);

	//Combat
	ATTRIBUTE_ACCESSORS(ThisClass, AttackRange);
	ATTRIBUTE_ACCESSORS(ThisClass, AttackRadius);
	ATTRIBUTE_ACCESSORS(ThisClass, AttackSpeed);

	ATTRIBUTE_ACCESSORS(ThisClass, BaseDamage);
	ATTRIBUTE_ACCESSORS(ThisClass, BaseDefence);

	ATTRIBUTE_ACCESSORS(ThisClass, CriticalHitChance);
	ATTRIBUTE_ACCESSORS(ThisClass, CriticalHitMultiplier);


	//Monster Only


protected:
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;


public:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData Health;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MaxHealth;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData HealthRegeneration;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData BaseDamage;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData BaseDefence;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData CriticalHitChance;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData CriticalHitMultiplier;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData AttackRange;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData AttackRadius;



	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData AttackSpeed;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MoveSpeed;

};

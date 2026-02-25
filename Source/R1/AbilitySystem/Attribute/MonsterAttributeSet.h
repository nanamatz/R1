

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "MonsterAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
/**
 * 
 */
UCLASS()
class R1_API UMonsterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	UMonsterAttributeSet();

	//Monster Only

	ATTRIBUTE_ACCESSORS(ThisClass, Xp);
	ATTRIBUTE_ACCESSORS(ThisClass, AggroRange);
	ATTRIBUTE_ACCESSORS(ThisClass, AttackAngle);


protected:
	//virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	//virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
//	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

private:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData Xp;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData AggroRange;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData AttackAngle;
};

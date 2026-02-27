


#include "AbilitySystem/R1DamageExecutionCalc.h"
#include "Attribute/PlayerAttributeSet.h"
#include "Attribute/R1AttributeSet.h"
#include "R1AbilitySystemComponent.h"

struct R1DamageStatics
{
// 공통 스탯
	DECLARE_ATTRIBUTE_CAPTUREDEF(BaseDamage);
	DECLARE_ATTRIBUTE_CAPTUREDEF(BaseDefence);

	// 플레이어 전용 스탯
	DECLARE_ATTRIBUTE_CAPTUREDEF(WeaponDamage);
	DECLARE_ATTRIBUTE_CAPTUREDEF(DamageMultiplier);
	DECLARE_ATTRIBUTE_CAPTUREDEF(EquipDefence);
	DECLARE_ATTRIBUTE_CAPTUREDEF(DefenceMultiplier);

	R1DamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UR1AttributeSet, BaseDamage, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UPlayerAttributeSet, WeaponDamage, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UPlayerAttributeSet, DamageMultiplier, Source, false);

		DEFINE_ATTRIBUTE_CAPTUREDEF(UR1AttributeSet, BaseDefence, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UPlayerAttributeSet, EquipDefence, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UPlayerAttributeSet, DefenceMultiplier, Target, false);
	}
};

static const R1DamageStatics& DamageStatics()
{
	static R1DamageStatics Statics;
	return Statics;
}

UR1DamageExecutionCalc::UR1DamageExecutionCalc()
{
	RelevantAttributesToCapture.Add(DamageStatics().BaseDamageDef);
	RelevantAttributesToCapture.Add(DamageStatics().WeaponDamageDef);
	RelevantAttributesToCapture.Add(DamageStatics().DamageMultiplierDef);

	RelevantAttributesToCapture.Add(DamageStatics().BaseDefenceDef);
	RelevantAttributesToCapture.Add(DamageStatics().EquipDefenceDef);
	RelevantAttributesToCapture.Add(DamageStatics().DefenceMultiplierDef);
}

void UR1DamageExecutionCalc::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = ExecutionParams.GetOwningSpec().CapturedSourceTags.GetAggregatedTags();
	EvaluationParameters.TargetTags = ExecutionParams.GetOwningSpec().CapturedTargetTags.GetAggregatedTags();

	float BaseDamage = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BaseDamageDef, EvaluationParameters, BaseDamage);

	float WeaponDamage = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().WeaponDamageDef, EvaluationParameters, WeaponDamage);

	float DamageMultiplier = 1.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DamageMultiplierDef, EvaluationParameters, DamageMultiplier);
	if (DamageMultiplier <= 0.0f) DamageMultiplier = 1.0f;

	float BaseDefence = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BaseDefenceDef, EvaluationParameters, BaseDefence);

	float EquipDefence = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().EquipDefenceDef, EvaluationParameters, EquipDefence);

	float DefenceMultiplier = 1.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DefenceMultiplierDef, EvaluationParameters, DefenceMultiplier);
	//if (DefenceMultiplier <= 0.0f) DefenceMultiplier = 1.0f;

	float TotalAttackPower = (BaseDamage + WeaponDamage) * DamageMultiplier;
	float TotalDefencePower = (BaseDefence + EquipDefence) * DefenceMultiplier;

	float MitigatedDamage = TotalAttackPower - TotalDefencePower;

	// 데미지가 음수가 되지 않도록 방어
	MitigatedDamage = FMath::Max<float>(MitigatedDamage, 0.0f);

	if (MitigatedDamage > 0.0f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			UR1AttributeSet::GetHealthAttribute(),
			EGameplayModOp::Additive,
			-MitigatedDamage
		));
	}
}




#include "AbilitySystem/R1DamageExecutionCalc.h"
#include "Attribute/PlayerAttributeSet.h"
#include "Attribute/R1AttributeSet.h"
#include "R1AbilitySystemComponent.h"
#include "R1GameplayTags.h"

struct R1DamageStatics
{
// 공통 스탯
	DECLARE_ATTRIBUTE_CAPTUREDEF(BaseDamage);
	DECLARE_ATTRIBUTE_CAPTUREDEF(BaseDefence);

	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitMultiplier);

	// 플레이어 전용 스탯
	DECLARE_ATTRIBUTE_CAPTUREDEF(WeaponDamage);
	DECLARE_ATTRIBUTE_CAPTUREDEF(DamageMultiplier);
	DECLARE_ATTRIBUTE_CAPTUREDEF(EquipDefence);
	DECLARE_ATTRIBUTE_CAPTUREDEF(DefenceMultiplier);

	R1DamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UR1AttributeSet, BaseDamage, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UR1AttributeSet, CriticalHitChance, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UR1AttributeSet, CriticalHitMultiplier, Source, false);

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
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitMultiplierDef);

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

	float SkillDamage = ExecutionParams.GetOwningSpec().GetSetByCallerMagnitude(R1GameplayTags::Data_Skill_Magnitude, false, 0.0f);
	
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

	float TotalAttackPower = (BaseDamage + WeaponDamage + SkillDamage) * DamageMultiplier;
	float TotalDefencePower = (BaseDefence + EquipDefence) * DefenceMultiplier;

	float MitigatedDamage = TotalAttackPower - TotalDefencePower;

	float CriticalHitChance = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitChanceDef, EvaluationParameters, CriticalHitChance);

	float CriticalHitMultiplier = 2.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitMultiplierDef, EvaluationParameters, CriticalHitMultiplier);
	CriticalHitMultiplier = FMath::Max<float>(CriticalHitMultiplier, 1.0f);

	bool bIsCriticalHit = false;
	if (CriticalHitChance > 0.0f)
	{
		float RandomRoll = FMath::FRand();
		if (RandomRoll <= CriticalHitChance)
		{
			bIsCriticalHit = true;
			MitigatedDamage *= CriticalHitMultiplier;

			// Inject dynamic tag to signal critical hit to PostGameplayEffectExecute
			FGameplayEffectSpec* MutableSpec = const_cast<FGameplayEffectSpec*>(&ExecutionParams.GetOwningSpec());
			if (MutableSpec)
			{
				MutableSpec->AddDynamicAssetTag(R1GameplayTags::Event_Hit_Critical);
			}
		}
	}

	// Apply ±25% Randomness (0.75 to 1.25)
	float RandomDeviation = FMath::RandRange(0.75f, 1.25f);
	MitigatedDamage *= RandomDeviation;

	// 최종 데미지를 정수로 변환
	int32 FinalDamage = FMath::Max<int32>(FMath::FloorToInt(MitigatedDamage), 0);

	if (FinalDamage > 0)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			UR1AttributeSet::GetHealthAttribute(),
			EGameplayModOp::Additive,
			-static_cast<float>(FinalDamage)
		));
	}
}

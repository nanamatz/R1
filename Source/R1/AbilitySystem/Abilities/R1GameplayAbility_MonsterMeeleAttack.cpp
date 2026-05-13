#include "AbilitySystem/Abilities/R1GameplayAbility_MonsterMeeleAttack.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/R1Character.h"
#include "Character/R1Player.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"

UR1GameplayAbility_MonsterMeeleAttack::UR1GameplayAbility_MonsterMeeleAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UR1GameplayAbility_MonsterMeeleAttack::OnAttackEventReceived(FGameplayEventData Payload)
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	AR1Character* SourceCharacter = Cast<AR1Character>(AvatarActor);

	if (!SourceCharacter) return;

	if (SoundToPlay)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, SourceCharacter->GetActorLocation());
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();

	if (DamageEffect && SourceASC)
	{
		FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, 1, EffectContext);
		if (EffectSpecHandle.IsValid() && EffectSpecHandle.Data.IsValid())
		{
			CheckAndApplyDamage_Sector(EffectSpecHandle, SourceCharacter, SourceASC);
		}
	}
}

void UR1GameplayAbility_MonsterMeeleAttack::CheckAndApplyDamage_Sector(const FGameplayEffectSpecHandle& SpecHandle, AR1Character* SourceCharacter, UAbilitySystemComponent* SourceASC)
{
	if (SourceCharacter == nullptr || SourceASC == nullptr)
	{
		return;
	}

	if (SpecHandle.IsValid() == false || SpecHandle.Data.IsValid() == false)
	{
		return;
	}

	UWorld* World = SourceCharacter->GetWorld();
	if (World == nullptr)
	{
		return;
	}

	float AttackRange = SourceASC->GetNumericAttribute(UR1AttributeSet::GetAttackRangeAttribute());
	float AttackRadius = SourceASC->GetNumericAttribute(UR1AttributeSet::GetAttackRadiusAttribute());
	if (AttackRange <= KINDA_SMALL_NUMBER || AttackRadius <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(SourceCharacter);

	bool bResult = World->OverlapMultiByChannel(
		OverlapResults,
		SourceCharacter->GetActorLocation(),
		FQuat::Identity,
		ECC_GameTraceChannel2, 
		FCollisionShape::MakeSphere(AttackRange),
		Params
	);

	if (!bResult)
	{
		return;
	}

	// 2. 각도 계산 (Dot Product)
	float DotThreshold = FMath::Cos(FMath::DegreesToRadians(AttackRadius / 2.0f));

	FVector MyForward = SourceCharacter->GetActorForwardVector();
	TSet<AActor*> ProcessedActors;

	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* TargetActor = Result.GetActor();
		if (!TargetActor) continue;

		if (ProcessedActors.Contains(TargetActor))
		{
			continue; // 이미 처리한 액터는 건너뜜
		}

		// 내적 계산
		AR1Player* TargetPlayer = Cast<AR1Player>(TargetActor);
		if (!TargetPlayer) continue;

		FVector DirToTarget = (TargetPlayer->GetActorLocation() - SourceCharacter->GetActorLocation()).GetSafeNormal();
		float DotResult = FVector::DotProduct(MyForward, DirToTarget);

		// 각도 안에 있다면
		if (DotResult >= DotThreshold)
		{
			// 데미지 적용
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetPlayer);
			if (TargetASC)
			{
				SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
				ProcessedActors.Add(TargetPlayer);
			}
		}
	}
}

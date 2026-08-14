#include "AbilitySystem/Abilities/R1GameplayAbility_WaveAttack.h"
#include "R1LogChannels.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Character/R1Player.h"
#include "Data/R1TelegraphData.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "R1GameplayTags.h"
#include "Particles/ParticleSystemComponent.h"
#include "Character/R1Monster.h"

void UR1GameplayAbility_WaveAttack::OnAttackEventReceived(FGameplayEventData Payload)
{
	AR1Monster* AvatarActor = Cast<AR1Monster>(GetAvatarActorFromActorInfo());
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();

	if (!AvatarActor || !SourceASC || !TelegraphData || !DamageEffect)
	{
		return;
	}

	FVector BossLoc = AvatarActor->GetActorLocation();
	FVector BossForward = AvatarActor->GetActorForwardVector();

	float Length = TelegraphData->TelegraphSize.X;
	float Width = TelegraphData->TelegraphSize.Y;

	// The Box Overlap center should be Half-Length in front of the boss
	FVector BoxCenter = BossLoc + (BossForward * (Length / 2.0f));
	// Box Extent is half-size in each dimension
	FVector BoxExtent = FVector(Length / 2.0f, Width / 2.0f, 100.0f);

	// Create Effect Spec
	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, 1, EffectContext);

	if (EffectSpecHandle.IsValid())
	{
		EffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(R1GameplayTags::Data_Skill_Magnitude, CachedDamage);
	}

	if (!EffectSpecHandle.IsValid())
	{
		return;
	}

	// Spawn Projectile
	FVector MuzzleLocation = AvatarActor->GetMesh()->GetSocketLocation(AvatarActor->MuzzleSocketName);

	// Direction: For now, use character forward. In a real game, you might want to aim at the target.
	FRotator MuzzleRotation = AvatarActor->GetActorRotation();

	if (WaveEffect)
	{
		// 스폰 후 컴포넌트 포인터를 받아옵니다.
		UParticleSystemComponent* ParticleComp = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			WaveEffect,
			MuzzleLocation,
			MuzzleRotation
		);

		if (ParticleComp)
		{
			FVector BeamEndLocation = MuzzleLocation + (BossForward * Length);
			ParticleComp->SetVectorParameter(FName("BeamTarget"), BeamEndLocation);
		}
	}

	// 데미지는 VFX 유무와 무관하게 항상 적용한다.
	// (이전에는 이 블록 전체가 if (WaveEffect) 안에 있어 파티클 미지정 시 무피해였다.)
	{
		// 1. 회전이 적용된 박스 형태(Collision Shape)와 방향(Quat) 설정
		FCollisionShape BoxShape = FCollisionShape::MakeBox(BoxExtent);
		FQuat BoxRotation = AvatarActor->GetActorQuat(); // 보스의 회전값을 그대로 적용!

		// 2. 감지할 오브젝트 타입 설정 (Pawn)
		FCollisionObjectQueryParams ObjectQueryParams;
		ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

		// 3. 무시할 대상 설정 (자기 자신)
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(AvatarActor);

		// 4. 네이티브 Overlap 실행 (회전값 적용)
		TArray<FOverlapResult> OverlapResults;
		bool bHit = GetWorld()->OverlapMultiByObjectType(
			OverlapResults,
			BoxCenter,
			BoxRotation, // 핵심: 여기서 박스를 보스가 바라보는 방향으로 회전시킵니다.
			ObjectQueryParams,
			BoxShape,
			QueryParams
		);

		// 5. 결과 처리 및 데미지 적용
		if (bHit)
		{
			TSet<AActor*> ProcessedActors; // 중복 타격 방지용 Set

			for (const FOverlapResult& Result : OverlapResults)
			{
				AActor* OverlappedActor = Result.GetActor();

				// 유효한 액터인지, 플레이어 클래스인지, 이미 맞은 적은 아닌지 검사
				if (OverlappedActor && OverlappedActor->IsA<AR1Player>() && !ProcessedActors.Contains(OverlappedActor))
				{
					ProcessedActors.Add(OverlappedActor);

					UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OverlappedActor);
					if (TargetASC)
					{
						// 데미지 적용
						SourceASC->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(), TargetASC);

						// [VFX] 피해를 준 플레이어 위치에 무기 임팩트 큐 실행 (어빌리티 지정 VFX를 SourceObject로 전달)
						FGameplayCueParameters CueParams;
						CueParams.SourceObject = GetHitImpactEffect();
						CueParams.Instigator = AvatarActor;
						CueParams.Location = OverlappedActor->GetActorLocation() + FVector(0, 0, 50.0f); // 명치 높이 보정
						CueParams.Normal = (BossLoc - OverlappedActor->GetActorLocation()).GetSafeNormal();
						SourceASC->ExecuteGameplayCue(R1GameplayTags::GameplayCue_Weapon_Impact, CueParams);
					}
				}
			}
			UE_LOG(LogR1, Log, TEXT("WaveAttack (Laser): Hit %d unique players"), ProcessedActors.Num());
		}
	}
}

#include "AbilitySystem/R1GameplayCueNotify_WeaponImpact.h"
#include "Character/R1Player.h"
#include "System/R1EquipmentManagerComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

bool UR1GameplayCueNotify_WeaponImpact::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	// 정적 큐는 CDO에서 실행되므로 월드는 대상 액터에서 얻는다
	if (!MyTarget || !MyTarget->GetWorld())
	{
		return false;
	}

	// [사운드] 기존 GCN_WeaponImpact BP 로직 이관: SourceObject의 무기 사운드 재생 (없으면 무음)
	if (USoundBase* Sound = Cast<USoundBase>(const_cast<UObject*>(Parameters.GetSourceObject())))
	{
		UGameplayStatics::PlaySoundAtLocation(MyTarget, Sound, Parameters.Location);
	}

	// [VFX] 우선순위: 어빌리티 지정(SourceObject의 Niagara) → 장착 무기 DA → DefaultHitVFX
	// (플레이어 기본 공격은 SourceObject가 사운드라 캐스트 실패 → 무기 DA 경로로 폴백)
	UNiagaraSystem* ImpactVFX = Cast<UNiagaraSystem>(const_cast<UObject*>(Parameters.GetSourceObject()));
	if (!ImpactVFX)
	{
		if (AR1Player* Player = Cast<AR1Player>(Parameters.GetInstigator()))
		{
			if (UR1EquipmentManagerComponent* EquipManager = Player->GetEquipmentComponent())
			{
				ImpactVFX = EquipManager->GetHitImpactVFX(ER1EquipmentSlot::Weapon);
			}
		}
	}
	if (!ImpactVFX)
	{
		ImpactVFX = DefaultHitVFX;
	}

	if (ImpactVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			MyTarget, ImpactVFX, Parameters.Location, Parameters.Normal.Rotation());
	}

	return false;
}


#include "AbilitySystem/R1GameplayCueNotify_AttachedVFX.h"
#include "R1LogChannels.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"

AR1GameplayCueNotify_AttachedVFX::AR1GameplayCueNotify_AttachedVFX()
{
	bAutoDestroyOnRemove = true;
}

bool AR1GameplayCueNotify_AttachedVFX::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	if (!NiagaraSystem)
	{
		UE_LOG(LogR1, Warning, TEXT("[%s] NiagaraSystem이 설정되지 않았습니다."), *GetName());
		return false;
	}

	// 같은 큐가 중복 활성화돼도(예: 버프 재적용) 이펙트는 하나만 유지
	if (SpawnedVFX && SpawnedVFX->IsActive())
	{
		return Super::OnActive_Implementation(MyTarget, Parameters);
	}

	// 스켈레탈 메시 샘플링(Attach Parent)이 동작하려면 반드시 메시 컴포넌트에 부착해야 한다.
	USceneComponent* AttachTo = nullptr;
	if (ACharacter* TargetChar = Cast<ACharacter>(MyTarget))
	{
		AttachTo = TargetChar->GetMesh();
	}
	if (!AttachTo && MyTarget)
	{
		AttachTo = MyTarget->GetRootComponent();
	}

	if (!AttachTo)
	{
		UE_LOG(LogR1, Warning, TEXT("[%s] 부착할 컴포넌트를 찾지 못했습니다. 대상: %s"), *GetName(), *GetNameSafe(MyTarget));
		return false;
	}

	SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
		NiagaraSystem,
		AttachTo,
		NAME_None,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::SnapToTarget,
		/*bAutoDestroy=*/false
	);

	UE_LOG(LogR1, Log, TEXT("[%s] VFX 부착: %s → %s(%s)"),
		*GetName(), *GetNameSafe(NiagaraSystem), *GetNameSafe(MyTarget), *GetNameSafe(AttachTo));

	return Super::OnActive_Implementation(MyTarget, Parameters);
}

bool AR1GameplayCueNotify_AttachedVFX::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	if (SpawnedVFX)
	{
		SpawnedVFX->DestroyComponent();
		SpawnedVFX = nullptr;
	}
	return Super::OnRemove_Implementation(MyTarget, Parameters);
}

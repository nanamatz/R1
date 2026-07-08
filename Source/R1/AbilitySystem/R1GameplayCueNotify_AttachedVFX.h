
#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "R1GameplayCueNotify_AttachedVFX.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

/**
 * 지속형 상태이상 큐: 큐 추가 시 대상 캐릭터의 스켈레탈 메시에 나이아가라 시스템을 부착하고,
 * 큐 제거 시 함께 파괴한다. (스켈레탈 메시 서페이스 샘플링의 Attach Parent가 동작하도록 메시에 직접 부착)
 * BP 자식(GCN_Status_Burning 등)은 GameplayCue Tag와 NiagaraSystem만 설정하면 된다.
 */
UCLASS()
class R1_API AR1GameplayCueNotify_AttachedVFX : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	AR1GameplayCueNotify_AttachedVFX();

protected:
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

	// 대상 메시에 부착할 루프 이펙트
	UPROPERTY(EditDefaultsOnly, Category = "R1|VFX")
	TObjectPtr<UNiagaraSystem> NiagaraSystem;

private:
	// 부착한 컴포넌트의 소유자는 대상 액터이므로, 큐 액터 파괴만으로는 정리되지 않는다 → OnRemove에서 직접 파괴
	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> SpawnedVFX;
};

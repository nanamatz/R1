#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "R1Define.h"
#include "R1DamageTextActor.generated.h"

UCLASS()
class R1_API AR1DamageTextActor : public AActor
{
	GENERATED_BODY()

public:
	AR1DamageTextActor();

	void SetDamageInfo(const FR1DamageInfo& DamageInfo);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class USceneComponent> RootComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UWidgetComponent> DamageTextWidgetComp;

	// 플로팅 애니메이션이 끝난 뒤 자동 파괴되기까지의 수명 (FloatAnim 길이보다 길게)
	UPROPERTY(EditDefaultsOnly, Category = "Damage UI")
	float LifeSpanSeconds = 1.5f;
};

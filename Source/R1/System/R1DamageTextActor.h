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

private:
	UFUNCTION()
	void HandleAnimationFinished();
};
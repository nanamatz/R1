

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/R1InteractionInterface.h"
#include "Interface/R1HighlightInterface.h"
#include "R1Dispenser.generated.h"

UCLASS()
class R1_API AR1Dispenser : public AActor, public IR1InteractionInterface, public IR1HighlightInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AR1Dispenser();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UBoxComponent> InteractionTrigger;

	// 회복용 GE 클래스 (에디터에서 설정)
	UPROPERTY(EditAnywhere, Category = "Recovery")
	TSubclassOf<class UGameplayEffect> RecoveryEffectClass;

	UPROPERTY(EditAnywhere, Category = "Recovery")
	TSubclassOf<AActor> DispenserParticleEffectClass;

	// 상호작용 인터페이스 구현
	virtual void Highlight() override;
	virtual void UnHighlight() override;
	virtual void Interact_Implementation(AR1PlayerController* Interactor) override;
	virtual UPrimitiveComponent* GetInteractTrigger() override;

private:
	bool bIsUsed = false;
};

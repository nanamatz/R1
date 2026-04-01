

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/R1InteractionInterface.h"
#include "Interface/R1HighlightInterface.h"
#include "R1LockDoor.generated.h"

UCLASS()
class R1_API AR1LockDoor : public AActor, public IR1InteractionInterface, public IR1HighlightInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AR1LockDoor();
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> LockDoorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UBoxComponent> TriggerBox;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Door Link")
	TObjectPtr<class AR1Door> LinkedMainDoor;

	UPROPERTY(EditAnywhere, Category = "Door Settings")
	float OpenSpeed = 2.0f;

	// 메인 문이 지시할 때 수행할 함수들
public:
	void SetLockVisibility(bool bIsVisible);
	void OpenDoorSmoothly();

	// Interfaces
	virtual void Highlight() override;
	virtual void UnHighlight() override;
	virtual void Interact_Implementation(AR1PlayerController* Interactor) override;
	virtual UPrimitiveComponent* GetInteractTrigger() override;

private:
	bool bIsOpening = false;
	FRotator TargetRotation;

};

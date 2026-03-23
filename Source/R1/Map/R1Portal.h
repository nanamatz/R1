

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/R1HighlightInterface.h"
#include "Interface/R1InteractionInterface.h"
#include "R1Portal.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class R1_API AR1Portal : public AActor, public IR1HighlightInterface, public IR1InteractionInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AR1Portal();

	virtual void Interact_Implementation(class AR1PlayerController* Interactor) override;
protected:
	virtual void BeginPlay() override;
	virtual void Highlight() override;
	virtual void UnHighlight() override;
	virtual UPrimitiveComponent* GetInteractTrigger() override;

public:
	// 문의 루트 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> RootComp;

	// 문의 외형 (메시)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PortalMesh;

	// 플레이어 접근을 감지할 트리거 박스
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerBox;
};

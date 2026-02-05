

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/R1HighlightInterface.h"
#include "InteractableActor.generated.h"

UCLASS()
class R1_API AInteractableActor : public AActor, public IR1HighlightInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractableActor();

private:
	virtual void Highlight() override;
	virtual void UnHighlight() override;

protected:
	bool bHighlighted = false;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UBoxComponent> BoxComponent;

};

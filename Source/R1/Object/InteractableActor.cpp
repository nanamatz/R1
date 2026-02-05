


#include "Object/InteractableActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "InteractableActor.h"

// Sets default values
AInteractableActor::AInteractableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Tags.Add(FName("Interactable"));

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	RootComponent = BoxComponent;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(BoxComponent);

}


void AInteractableActor::Highlight()
{
	if (Mesh)
	{
		bHighlighted = true;
		Mesh->SetRenderCustomDepth(true);
		Mesh->SetCustomDepthStencilValue(225);
	}
	else
	{
		return;
	}
}

void AInteractableActor::UnHighlight()
{
	if (Mesh)
	{
		bHighlighted = false;
		Mesh->SetRenderCustomDepth(false);
	}
	else
	{
		return;
	}
}


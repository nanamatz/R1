


#include "Object/R1Dispenser.h"
#include "R1LogChannels.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/R1Player.h"
#include "Player/R1PlayerController.h"
#include "AbilitySystem/R1AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "AbilitySystem/Attribute/PlayerAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "UI/R1HUD.h"
#include "Data/R1UISoundData.h"

// Sets default values
AR1Dispenser::AR1Dispenser()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	SetRootComponent(MeshComp);
	MeshComp->SetCollisionProfileName(TEXT("BlockAll"));

	InteractionTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionTrigger"));
	InteractionTrigger->SetupAttachment(RootComponent);

	InteractionTrigger->SetCollisionProfileName(TEXT("Trigger"));
	Tags.Add(FName("Interactable"));

}

// Called when the game starts or when spawned
void AR1Dispenser::BeginPlay()
{
	Super::BeginPlay();
	
}

void AR1Dispenser::Highlight()
{
	if (bIsUsed) return;

	if (MeshComp)
	{
		MeshComp->SetRenderCustomDepth(true);
		MeshComp->SetCustomDepthStencilValue(252);
	}
}

void AR1Dispenser::UnHighlight()
{
	if (MeshComp)
	{
		MeshComp->SetRenderCustomDepth(false);
	}
}

void AR1Dispenser::Interact_Implementation(AR1PlayerController* Interactor)
{
	if (!Interactor || bIsUsed) return;

	AR1Player* Player = Cast<AR1Player>(Interactor->GetCharacter());
	if (!Player) return;

	UR1AbilitySystemComponent* ASC = Cast<UR1AbilitySystemComponent>(Player->GetAbilitySystemComponent());
	if (!ASC) return;

	const UR1AttributeSet* BaseSet = ASC->GetSet<UR1AttributeSet>();
	const UPlayerAttributeSet* PlayerSet = ASC->GetSet<UPlayerAttributeSet>();

	if (!BaseSet || !PlayerSet) return;

	float CurHP = BaseSet->GetHealth();
	float MaxHP = BaseSet->GetMaxHealth();
	float CurMP = PlayerSet->GetMana();
	float MaxMP = PlayerSet->GetMaxMana();

	if (CurHP >= MaxHP && CurMP >= MaxMP)
	{
		UE_LOG(LogR1, Warning, TEXT("이미 체력과 마나가 가득 차 있습니다!"));

		if (RecoveryErrorSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, RecoveryErrorSound, GetActorLocation());
		}
		else if (AR1HUD* HUD = Cast<AR1HUD>(Interactor->GetHUD()))
		{
			if (HUD->UISoundData && HUD->UISoundData->ActionError)
			{
				UGameplayStatics::PlaySoundAtLocation(this, HUD->UISoundData->ActionError, GetActorLocation());
			}
		}

		return;
	}

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddInstigator(Player, Player);

	if ((CurHP < MaxHP  || CurMP < MaxMP) && RecoveryEffectClass)
	{
		FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(RecoveryEffectClass, 1.0f, Context);
		ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}

	if (DispenserParticleEffectClass)
	{
		FVector SpawnLocation = Player->GetActorLocation();
		FRotator SpawnRotation = Player->GetActorRotation();
		GetWorld()->SpawnActor<AActor>(DispenserParticleEffectClass, SpawnLocation, SpawnRotation);
	}

	if (RecoverySound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, RecoverySound, GetActorLocation());
	}

	bIsUsed = true;
	UnHighlight();
}

UPrimitiveComponent* AR1Dispenser::GetInteractTrigger()
{
	return InteractionTrigger;
}


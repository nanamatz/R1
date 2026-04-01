


#include "Object/R1Dispenser.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/R1Player.h"
#include "Player/R1PlayerController.h"
#include "AbilitySystem/R1AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "AbilitySystem/Attribute/PlayerAttributeSet.h"

// Sets default values
AR1Dispenser::AR1Dispenser()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	SetRootComponent(MeshComp);

	InteractionTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionTrigger"));
	InteractionTrigger->SetupAttachment(RootComponent);

	InteractionTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionTrigger->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	InteractionTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
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

	// 🌟 1. 현재 체력/마나 상태 확인
	// 유저님이 작성하신 AttributeSet에서 값을 가져옵니다.
	const UR1AttributeSet* BaseSet = ASC->GetSet<UR1AttributeSet>();
	const UPlayerAttributeSet* PlayerSet = ASC->GetSet<UPlayerAttributeSet>();

	if (!BaseSet || !PlayerSet) return;

	float CurHP = BaseSet->GetHealth();
	float MaxHP = BaseSet->GetMaxHealth();
	float CurMP = PlayerSet->GetMana();
	float MaxMP = PlayerSet->GetMaxMana();

	// 🌟 2. 둘 다 가득 차 있다면 상호작용 거부 (횟수 차감 없음)
	if (CurHP >= MaxHP && CurMP >= MaxMP)
	{
		UE_LOG(LogTemp, Warning, TEXT("이미 체력과 마나가 가득 차 있습니다!"));
		return;
	}

	// 🌟 3. 회복 GE 적용
	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddInstigator(Player, Player);

	// 체력이 부족하면 체력 회복 GE 적용
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
	// 🌟 4. 사용 완료 처리 (일회성)
	bIsUsed = true;
	UnHighlight();

	if (UsedMaterial)
	{
		MeshComp->SetMaterial(0, UsedMaterial);
	}

	UE_LOG(LogTemp, Warning, TEXT("디스펜서 사용 완료: 연료 고갈"));
}

UPrimitiveComponent* AR1Dispenser::GetInteractTrigger()
{
	return InteractionTrigger;
}


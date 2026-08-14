

#include "Map/TeleportActor.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Character/R1Player.h"
#include "Player/R1PlayerController.h"

ATeleportActor::ATeleportActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	TeleportMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TeleportMesh"));
	TeleportMesh->SetupAttachment(RootComp);
	TeleportMesh->SetCollisionProfileName(TEXT("BlockAll"));

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComp);

	// 커서 트레이스(Attack 채널)를 Block해야 하이라이트/클릭이 동작함 (AR1Portal과 동일 패턴)
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetBoxExtent(FVector(150.0f, 150.0f, 100.0f));

	ForwardArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("ForwardArrow"));
	ForwardArrow->SetupAttachment(RootComp);
	ForwardArrow->ArrowSize = 2.0f;

	Tags.Add(FName("Interactable"));
}

void ATeleportActor::BeginPlay()
{
	Super::BeginPlay();

	// 한쪽에만 목적지를 지정해도 양방향으로 동작하도록 상대편의 빈 링크를 채워준다.
	// (상대가 이미 다른 텔레포터를 가리키고 있으면 그 설정을 존중한다)
	if (LinkedTeleporter && LinkedTeleporter != this && LinkedTeleporter->LinkedTeleporter == nullptr)
	{
		LinkedTeleporter->LinkedTeleporter = this;
	}
}

FVector ATeleportActor::GetExitLocation() const
{
	return GetActorLocation() + GetActorForwardVector() * ExitOffset;
}

bool ATeleportActor::IsOnCooldown() const
{
	return GetWorld() && GetWorld()->GetTimeSeconds() < CooldownEndTime;
}

void ATeleportActor::StartCooldown()
{
	if (TeleportCooldown <= 0.0f) return;

	CooldownEndTime = GetWorld()->GetTimeSeconds() + TeleportCooldown;
	RefreshHighlight();

	// 컨트롤러는 커서가 올라간 액터가 바뀔 때만 Highlight를 다시 호출하므로,
	// 계속 올려둔 상태로 쿨다운이 끝나면 색이 갱신되지 않는다. 종료 시점에 직접 갱신.
	GetWorldTimerManager().SetTimer(CooldownTimerHandle, this, &ATeleportActor::RefreshHighlight, TeleportCooldown, false);
}

void ATeleportActor::Interact_Implementation(AR1PlayerController* Interactor)
{
	if (!Interactor || !LinkedTeleporter) return;

	// 플레이어 전용
	AR1Player* Player = Cast<AR1Player>(Interactor->GetPawn());
	if (!Player) return;

	if (IsOnCooldown()) return;

	const FVector ExitLocation = LinkedTeleporter->GetExitLocation();

	if (TeleportSound)
	{
		UGameplayStatics::PlaySound2D(this, TeleportSound);
	}

	if (TeleportVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, TeleportVFX, GetActorLocation(), GetActorRotation());
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, TeleportVFX, ExitLocation, LinkedTeleporter->GetActorRotation());
	}

	// 이동 명령이 남아 있으면 도착 직후 원래 목적지로 다시 걸어가버림
	Interactor->StopMovement();
	if (UCharacterMovementComponent* MoveComp = Player->GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
	}

	// 위치 이동은 카메라 스냅 처리가 들어있는 TeleportToRoom을 재사용
	Player->TeleportToRoom(ExitLocation);

	// 목적지 텔레포터가 바라보는 방향으로 정면 고정 (Yaw만)
	const FRotator ExitRotation(0.0f, LinkedTeleporter->GetActorRotation().Yaw, 0.0f);
	Player->SetActorRotation(ExitRotation);
	Interactor->SetControlRotation(ExitRotation);

	// 도착지에서 곧바로 되돌아가는 핑퐁을 막기 위해 양쪽 모두 쿨다운
	StartCooldown();
	LinkedTeleporter->StartCooldown();
}

void ATeleportActor::Highlight()
{
	bHighlighted = true;

	if (TeleportMesh)
	{
		TeleportMesh->SetRenderCustomDepth(true);
		// 쿨다운 중이면 다른 색으로 표시해 "지금은 못 쓴다"를 알린다.
		TeleportMesh->SetCustomDepthStencilValue(IsOnCooldown() ? CooldownStencil : ReadyStencil);
	}
}

void ATeleportActor::UnHighlight()
{
	bHighlighted = false;

	if (TeleportMesh)
	{
		TeleportMesh->SetRenderCustomDepth(false);
	}
}

void ATeleportActor::RefreshHighlight()
{
	if (bHighlighted)
	{
		Highlight();
	}
}

UPrimitiveComponent* ATeleportActor::GetInteractTrigger()
{
	return TriggerBox;
}

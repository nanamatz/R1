


#include "Map/R1Door.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Character/R1Player.h"
#include "Item/R1InventorySubsystem.h"
#include "Player/R1PlayerController.h"

// Sets default values
AR1Door::AR1Door()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	BaseDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseDoorMesh"));
	BaseDoorMesh->SetupAttachment(RootComp);
	NoEntryMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NoEntryMesh"));
	NoEntryMesh->SetupAttachment(RootComp);
	LockDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LockDoorMesh"));
	LockDoorMesh->SetupAttachment(RootComp);

	SpecialDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpecialDoorMesh"));
	SpecialDoorMesh->SetupAttachment(RootComp);

	DoorwayHighlightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorwayHighlightMesh"));
	DoorwayHighlightMesh->SetupAttachment(RootComp);
	DoorwayHighlightMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DoorwayHighlightMesh->SetRenderCustomDepth(false);
	DoorwayHighlightMesh->SetCustomDepthStencilValue(252);

	BossDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BossDoorMesh"));
	BossDoorMesh->SetupAttachment(RootComp);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);

	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	Tags.Add(FName("Interactable"));
}

void AR1Door::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsOpening) return;

	bool bBaseMoving = false;
	bool bLockRotating = false;

	UStaticMeshComponent* ActiveMesh = nullptr;
	if (BaseDoorMesh && BaseDoorMesh->IsVisible()) ActiveMesh = BaseDoorMesh;
	else if (SpecialDoorMesh && SpecialDoorMesh->IsVisible()) ActiveMesh = SpecialDoorMesh;
	else if (BossDoorMesh && BossDoorMesh->IsVisible()) ActiveMesh = BossDoorMesh;

	if (ActiveMesh)
	{
		FVector CurrentLoc = ActiveMesh->GetRelativeLocation();
		FVector NewLoc = FMath::VInterpTo(CurrentLoc, TargetBaseLocation, DeltaTime, OpenSpeed);
		ActiveMesh->SetRelativeLocation(NewLoc);

		if (!NewLoc.Equals(TargetBaseLocation, 1.0f)) bBaseMoving = true;
	}

	// 2. LockDoor 보간 (Interact 시점에만 회전값이 세팅됨)
	// 여기서 !bRequiresKey 조건은 이미 풀렸을 때(열쇠 사용 후) 돌아가게 하기 위함입니다.
	if (LockDoorMesh && LockDoorMesh->IsVisible() && !bLocked)
	{
		FRotator CurrentRot = LockDoorMesh->GetRelativeRotation();
		FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetLockRotation, DeltaTime, OpenSpeed);
		LockDoorMesh->SetRelativeRotation(NewRot);

		if (!NewRot.Equals(TargetLockRotation, 1.0f)) bLockRotating = true;
	}

	if (!bBaseMoving && !bLockRotating)
	{
		bIsOpening = false;
		SetActorTickEnabled(false);
	}
}

void AR1Door::BeginPlay()
{
	Super::BeginPlay();

	SetActorTickEnabled(false);
}

void AR1Door::Highlight()
{
	if (!bIsOpened)
	{
		if (BaseDoorMesh && BaseDoorMesh->IsVisible())
		{
			BaseDoorMesh->SetRenderCustomDepth(true);
			BaseDoorMesh->SetCustomDepthStencilValue(252);
		}
		if (SpecialDoorMesh && SpecialDoorMesh->IsVisible())
		{
			SpecialDoorMesh->SetRenderCustomDepth(true);
			SpecialDoorMesh->SetCustomDepthStencilValue(252);
		}
		if (LockDoorMesh && LockDoorMesh->IsVisible())
		{
			LockDoorMesh->SetRenderCustomDepth(true);
			LockDoorMesh->SetCustomDepthStencilValue(252);
		}
		if (BossDoorMesh && BossDoorMesh->IsVisible())
		{
			BossDoorMesh->SetRenderCustomDepth(true);
			BossDoorMesh->SetCustomDepthStencilValue(252);
		}
	}
	else
	{
		if (DoorwayHighlightMesh)
		{
			// 게임엔 안 보이지만, 이 옵션을 켜면 포스트 프로세스가 외곽선만 그립니다!
			DoorwayHighlightMesh->SetRenderCustomDepth(true);
			DoorwayHighlightMesh->SetCustomDepthStencilValue(252);
		}
	}
}

void AR1Door::UnHighlight()
{
	if (BaseDoorMesh) BaseDoorMesh->SetRenderCustomDepth(false);
	if (SpecialDoorMesh) SpecialDoorMesh->SetRenderCustomDepth(false);
	if (LockDoorMesh) LockDoorMesh->SetRenderCustomDepth(false);
	if (DoorwayHighlightMesh) DoorwayHighlightMesh->SetRenderCustomDepth(false);
	if (BossDoorMesh) BossDoorMesh->SetRenderCustomDepth(false);
}

UPrimitiveComponent* AR1Door::GetInteractTrigger()
{
	return TriggerBox;
}

void AR1Door::ExecuteDoorTransition()
{
	OnDoorEntered.Broadcast(DoorDirection);
}

void AR1Door::SetupDoorConnection(int32 InTargetNodeID, ER1RoomContentType TargetRoomType)
{
	TargetNodeID = InTargetNodeID;

	LockDoorMesh->SetVisibility(false);
	LockDoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	NoEntryMesh->SetVisibility(false);
	NoEntryMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SpecialDoorMesh->SetVisibility(false);
	SpecialDoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	LockDoorMesh->SetVisibility(false);
	LockDoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	BaseDoorMesh->SetVisibility(false);
	BaseDoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	BossDoorMesh->SetVisibility(false);
	BossDoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (TargetNodeID == -1)
	{
		NoEntryMesh->SetVisibility(true);
		NoEntryMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
		BaseDoorMesh->SetVisibility(false);
		BaseDoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		DoorwayHighlightMesh->SetVisibility(false);
		DoorwayHighlightMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{
		SetActorEnableCollision(true);
		
		// 특수 방(상점, 보물방, 회복방)인 경우 메시 교체
		if (TargetRoomType == ER1RoomContentType::Shop || 
			TargetRoomType == ER1RoomContentType::Treasure || 
			TargetRoomType == ER1RoomContentType::Refresh)
		{
			SpecialDoorMesh->SetVisibility(true);
			SpecialDoorMesh->SetCollisionProfileName(TEXT("BlockAll"));
		}
		else if (TargetRoomType == ER1RoomContentType::Boss)
		{
			BossDoorMesh->SetVisibility(true);
			BossDoorMesh->SetCollisionProfileName(TEXT("BlockAll"));
		}
		else
		{
			BaseDoorMesh->SetVisibility(true);
			BaseDoorMesh->SetCollisionProfileName(TEXT("BlockAll"));
		}
	}
}

void AR1Door::SetLocked(bool bIsLocked)
{
	bCleared = !bIsLocked;
	//if (bLocked)
	//{
	//	// 문을 잠그는 로직 (예: 충돌 활성화, 머티리얼 변경 등)
	//	//BaseDoorMesh->SetCollisionProfileName(TEXT("BlockAll"));
	//	// TODO: 나중에 블루프린트에서 이 bLocked 변수를 읽고 '철창이 내려오는 애니메이션'을 재생하거나 '빨간색 빛'을 켤 수 있습니다.
	//}
	//else
	//{

	//}
}

void AR1Door::Interact_Implementation(AR1PlayerController* Interactor)
{
	if (!Interactor || !bCleared || TargetNodeID == -1) return;

	if (bRequiresKey)
	{
		UR1InventorySubsystem* Inven = GetWorld()->GetSubsystem<UR1InventorySubsystem>();

		if (Inven && Inven->ConsumeKeyItem())
		{
			bRequiresKey = false;
			bLocked = false;
			if (LockDoorMesh)
			{
				TargetLockRotation = LockDoorMesh->GetRelativeRotation() + FRotator(0.f, 90.f, 0.f);
				LockDoorMesh->SetCollisionProfileName(TEXT("NoCollision"));
				bIsOpening = true;
				SetActorTickEnabled(true);
			}
			GetWorld()->GetTimerManager().SetTimer(DoorTransitionTimer, this, &AR1Door::ExecuteDoorTransition, 0.5f, false);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("열쇠가 부족합니다!"));
			// 열쇠가 없다는 피드백 또는 열쇠가 필요하다는 피드백 효과 여기에 추가
		}
		return;
	}

	// 정상적으로 들어갈 수 있는 일반 문일 때
	OnDoorEntered.Broadcast(DoorDirection);
}

void AR1Door::SetKeyLocked(bool bNeedsKey)
{
	bRequiresKey = bNeedsKey;
	if (bRequiresKey)
	{
		bLocked = true;

		if (LockDoorMesh)
		{
			LockDoorMesh->SetVisibility(true);
			LockDoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
	}
}

void AR1Door::OpenDoor()
{
	if (bIsOpened) return;

	// 🌟 3. 현재 위치를 기준으로 '목표 지점'만 계산해서 저장합니다.
	if (BaseDoorMesh && BaseDoorMesh->IsVisible())
	{
		TargetBaseLocation = BaseDoorMesh->GetRelativeLocation() + FVector(0.f, -200.f, 0.f);
	}
	else if (SpecialDoorMesh && SpecialDoorMesh->IsVisible())
	{
		TargetBaseLocation = SpecialDoorMesh->GetRelativeLocation() + FVector(0.f, -200.f, 0.f);
	}
	else if (BossDoorMesh && BossDoorMesh->IsVisible())
	{
		TargetBaseLocation = BossDoorMesh->GetRelativeLocation() + FVector(0.f, -200.f, 0.f);
	}

	// 🌟 4. 이제부터 부드럽게 움직이라고 틱을 깨웁니다.
	bIsOpened = true;
	bIsOpening = true;
	SetActorTickEnabled(true);
}
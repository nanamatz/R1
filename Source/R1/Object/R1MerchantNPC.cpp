#include "Object/R1MerchantNPC.h"
#include "R1LogChannels.h"
#include "Data/R1ItemAssetData.h"
#include "Data/R1ItemPoolData.h"
#include "Data/R1ShopNPCData.h"
#include "Data/R1NPCPoolData.h"
#include "Item/R1ItemInstance.h"
#include "Character/R1Player.h"
#include "UI/R1HUD.h" 
#include "Kismet/GameplayStatics.h"
#include "Player/R1PlayerController.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "System/R1SaveSystem.h"
#include "Map/DungeonManager.h"
#include "EngineUtils.h"

AR1MerchantNPC::AR1MerchantNPC()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(RootComp);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComp->SetupAttachment(RootComp);
	MeshComp->SetCollisionProfileName(TEXT("BlockAll"));

	InteractTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractTrigger"));
	InteractTrigger->SetupAttachment(RootComp);
	InteractTrigger->SetCollisionProfileName(TEXT("Trigger")); // 오버랩만 감지하는 프로필

	Tags.Add(FName("Interactable"));
}

void AR1MerchantNPC::BeginPlay()
{
	Super::BeginPlay();

	// 스폰 시점에 아이템을 딱 한 번만 생성해서 고정합니다.
	GenerateShopItems();
	
	AssignNPCIdentity();

	if (InteractTrigger)
	{
		InteractTrigger->OnComponentEndOverlap.AddDynamic(this, &AR1MerchantNPC::OnInteractTriggerEndOverlap);
	}
}

void AR1MerchantNPC::OnInteractTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// 1. 범위를 벗어난 대상이 플레이어인지 확인합니다.
	if (AR1Player* Player = Cast<AR1Player>(OtherActor))
	{
		// 2. 플레이어의 컨트롤러를 통해 HUD에 접근합니다.
		if (AR1PlayerController* PC = Cast<AR1PlayerController>(Player->GetController()))
		{
			if (AR1HUD* HUD = Cast<AR1HUD>(PC->GetHUD()))
			{
				HUD->CloseShopUI();
			}
		}
	}
}

void AR1MerchantNPC::Highlight()
{
	if (MeshComp)
	{
		MeshComp->SetRenderCustomDepth(true);
		MeshComp->SetCustomDepthStencilValue(252); // 하이라이트 색상 설정 (프로젝트 설정에 따라 다름)
	}
}

void AR1MerchantNPC::UnHighlight()
{
	if (MeshComp)
	{
		MeshComp->SetRenderCustomDepth(false);
	}
}

void AR1MerchantNPC::Interact_Implementation(AR1PlayerController* Interactor)
{
	if (!Interactor) return;

	OpenShop();
}

UPrimitiveComponent* AR1MerchantNPC::GetInteractTrigger()
{
	return InteractTrigger;
}

void AR1MerchantNPC::OpenShop()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	if (AR1HUD* HUD = Cast<AR1HUD>(PC->GetHUD()))
	{
		HUD->OpenShopUI(this);
	}
}

bool AR1MerchantNPC::RemoveItemFromSale(UR1ItemInstance* ItemToRemove)
{
	if (ItemsForSale.Contains(ItemToRemove))
	{
		ItemsForSale.Remove(ItemToRemove);

		UR1SaveSystem* SaveSystem = GetGameInstance()->GetSubsystem<UR1SaveSystem>();
		int32 RoomID = GetRoomNodeID();

		if (SaveSystem && RoomID != -1)
		{
			SaveSystem->SaveShopInventory(RoomID, ItemsForSale);
		}

		return true;
	}
	return false;
}

void AR1MerchantNPC::AssignNPCIdentity()
{
	if (NPCPool && NPCPool->AvailableNPCs.Num() > 0)
	{
		// 풀 안에 있는 NPC 목록 중에서 랜덤하게 하나를 뽑아 내 신분으로 삼습니다.
		int32 RandomIndex = FMath::RandRange(0, NPCPool->AvailableNPCs.Num() - 1);
		CurrentNPCData = NPCPool->AvailableNPCs[RandomIndex];
	}
	else
	{
		UE_LOG(LogR1, Warning, TEXT("MerchantNPC: NPCPool이 할당되지 않았거나 비어 있습니다!"));
	}
}

void AR1MerchantNPC::GenerateShopItems()
{
	if (!ItemPool || ItemPool->DropItems.Num() == 0)
	{
		UE_LOG(LogR1, Warning, TEXT("Merchant ItemPool is empty or null!"));
		return;
	}

	UR1SaveSystem* SaveSystem = GetGameInstance()->GetSubsystem<UR1SaveSystem>();
	int32 RoomID = GetRoomNodeID();

	if (SaveSystem && RoomID != -1)
	{
		TArray<UR1ItemInstance*> CachedItems;
		if (SaveSystem->LoadShopInventory(RoomID, CachedItems, this))
		{
			ItemsForSale = CachedItems;
			UE_LOG(LogR1, Warning, TEXT("Merchant: %d번 방 상점 리스트 복원 완료!"), RoomID);
			return; // 복원 성공 시 새로 뽑지 않고 그대로 종료!
		}
	}

	if (ItemsForSale.IsEmpty())
	{
		TArray<TObjectPtr<UR1ItemAssetData>> AvailablePool = ItemPool->DropItems;

		int32 TargetCount = FMath::Min(CuratedItemCounts, AvailablePool.Num());

		while (ItemsForSale.Num() < TargetCount)
		{
			int32 RandomIndex = FMath::RandRange(0, AvailablePool.Num() - 1);
			UR1ItemAssetData* ChosenData = AvailablePool[RandomIndex];

			if (ChosenData)
			{
				UR1ItemInstance* NewItem = NewObject<UR1ItemInstance>(this);
				NewItem->Init(ChosenData, ChosenData->ItemRarity);
				NewItem->ItemCount = 1; // 기본 1개로 고정
				ItemsForSale.Add(NewItem);
			}

			// 중복 방지를 위해 뽑은 아이템은 풀에서 제거
			AvailablePool.RemoveAt(RandomIndex);
		}

		if (SaveSystem && RoomID != -1)
		{
			SaveSystem->SaveShopInventory(RoomID, ItemsForSale);
			UE_LOG(LogR1, Warning, TEXT("Merchant: %d번 방 상점 리스트 최초 생성 및 캐싱 완료!"), RoomID);
		}
	}
}

int32 AR1MerchantNPC::GetRoomNodeID()
{
	// 같은 스트리밍 레벨(방) 안에 존재하는 던전 매니저를 찾아서 그 방의 ID를 가져옵니다.
	for (TActorIterator<ADungeonManager> It(GetWorld()); It; ++It)
	{
		if (It->GetLevel() == this->GetLevel())
		{
			return It->RoomNodeID;
		}
	}
	return -1; // 못 찾았을 경우
}

#include "Object/R1MerchantNPC.h"
#include "Data/R1ItemAssetData.h"
#include "Data/R1ItemPoolData.h"
#include "Item/R1ItemInstance.h"
#include "UI/R1HUD.h" 
#include "Kismet/GameplayStatics.h"
#include "Player/R1PlayerController.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

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
		return true;
	}
	return false;
}

void AR1MerchantNPC::GenerateShopItems()
{
	if (!ItemPool || ItemPool->DropItems.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Merchant ItemPool is empty or null!"));
		return;
	}

	ItemsForSale.Empty();

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
}

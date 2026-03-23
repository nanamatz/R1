#include "Object/R1ItemActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/TextBlock.h"

#include "Character/R1Player.h"
#include "Item/R1ItemInstance.h"
#include "Item/R1InventorySubsystem.h"
#include "Data/R1ItemAssetData.h"
#include "Item/R1ItemTooltip.h"

#include "Player/R1PlayerController.h"

AR1ItemActor::AR1ItemActor()
{
	PrimaryActorTick.bCanEverTick = false;
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	RootComponent = SphereComp;
	SphereComp->SetSphereRadius(50.0f);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block); // 마우스 클릭용
	// 클릭을 감지해야 하므로 Visibility 채널 블록 설정 등 필요

	TooltipWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("TooltipWidget"));
	TooltipWidget->SetupAttachment(RootComponent);
	TooltipWidget->SetWidgetSpace(EWidgetSpace::Screen); // 화면에 항상 똑바로 보이게
	TooltipWidget->SetDrawAtDesiredSize(true);
	TooltipWidget->SetVisibility(false);


	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 메시는 충돌 끄기

	Tags.Add(FName("Item"));
}

void AR1ItemActor::BeginPlay()
{
	Super::BeginPlay();

}

void AR1ItemActor::Highlight()
{
	if (MeshComp)
	{
		MeshComp->SetRenderCustomDepth(true);
		MeshComp->SetCustomDepthStencilValue(251);
	}

	if (TooltipWidget)
	{
		TooltipWidget->SetVisibility(true); // 마우스 올리면 이름 표시!
	}

	bHighlighted = true;
}

void AR1ItemActor::UnHighlight()
{
	if (MeshComp)
	{
		MeshComp->SetRenderCustomDepth(false);
	}

	if (TooltipWidget)
	{
		TooltipWidget->SetVisibility(false); // 마우스 내리면 숨김!
	}

	bHighlighted = false;
}

void AR1ItemActor::Interact_Implementation(AR1PlayerController* Interactor)
{
	if (!Interactor) return;

	// 매개변수로 받은 플레이어 컨트롤러가 현재 조종 중인 캐릭터(Pawn)를 가져옵니다.
	if (AR1Player* PlayerCharacter = Cast<AR1Player>(Interactor->GetPawn()))
	{
		// 기존에 잘 만들어두신 루팅(획득) 함수를 그대로 호출!
		OnLootAttempted(PlayerCharacter);
	}
}

UPrimitiveComponent* AR1ItemActor::GetInteractTrigger()
{
	return SphereComp;
}

void AR1ItemActor::InitItem(UR1ItemAssetData* InItemData, EItemRarity InRarity, int32 InCount)
{
	// 🌟 데이터 테이블 검색 로직 전부 삭제! 던져준 에셋을 그대로 씁니다.
	ItemData = InItemData;
	ItemRarity = InRarity;
	ItemCount = InCount;

	if (ItemData && TooltipWidget)
	{
		if (ItemData->ItemMesh && MeshComp)
		{
			MeshComp->SetStaticMesh(ItemData->ItemMesh);
		}

		UR1ItemTooltip* TooltipUI = Cast<UR1ItemTooltip>(TooltipWidget->GetUserWidgetObject());

		if (TooltipUI)
		{
			// 데이터 에셋에 있는 아이템 이름을 UI로 쏴줍니다!
			TooltipUI->SetItemInfo(FText::FromName(ItemData->ItemName), ItemRarity, ItemCount, ItemData->ItemType, ItemData->BaseValue, false);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ItemActor: 전달받은 ItemData가 Null입니다!"));
	}
}

void AR1ItemActor::OnLootAttempted(AR1Player* Looter)
{
	if (!Looter || !ItemData) return;

	UR1InventorySubsystem* InvenSubsys = GetWorld()->GetSubsystem<UR1InventorySubsystem>();
	if (InvenSubsys && InvenSubsys->AddItem(ItemData, ItemRarity,ItemCount))
	{
		UE_LOG(LogTemp, Warning, TEXT("아이템 획득: %s"), *ItemData->ItemName.ToString());
		Destroy();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("인벤토리가 가득 차서 먹을 수 없습니다!"));
	}
}

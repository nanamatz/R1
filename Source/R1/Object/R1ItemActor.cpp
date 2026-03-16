


#include "Object/R1ItemActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Character/R1Player.h"
#include "Item/R1ItemInstance.h"
#include "Item/R1InventorySubsystem.h"
#include "Data/R1ItemAssetData.h"

AR1ItemActor::AR1ItemActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	RootComponent = SphereComp;
	SphereComp->SetSphereRadius(50.0f);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block); // 마우스 클릭용
	// 클릭을 감지해야 하므로 Visibility 채널 블록 설정 등 필요

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 메시는 충돌 끄기

	Tags.Add(FName("Item"));
}

void AR1ItemActor::BeginPlay()
{
	Super::BeginPlay();

}

void AR1ItemActor::InitItem(UR1ItemAssetData* InItemData, EItemRarity InRarity)
{
	// 🌟 데이터 테이블 검색 로직 전부 삭제! 던져준 에셋을 그대로 씁니다.
	ItemData = InItemData;
	ItemRarity = InRarity;

	if (ItemData)
	{
		if (ItemData->ItemMesh && MeshComp)
		{
			MeshComp->SetStaticMesh(ItemData->ItemMesh);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ItemActor: 전달받은 ItemData가 Null입니다!"));
	}
}

void AR1ItemActor::OnLootAttempted(AR1Player* Looter)
{
	if (!Looter || !ItemData) return; // 🌟 내 아이템 데이터가 없으면 패스

	UR1InventorySubsystem* InvenSubsys = GetWorld()->GetSubsystem<UR1InventorySubsystem>();
	if (!InvenSubsys) return;

	// 1. 임시 인스턴스 생성 및 데이터 에셋 주입 (초간단!)
	UR1ItemInstance* TempInstance = NewObject<UR1ItemInstance>(InvenSubsys);
	TempInstance->Init(ItemData); // 🌟 포인터만 넘겨줍니다!
	TempInstance->ItemRarity = ItemRarity; // 희귀도 정보도 잊지 않고 넘겨줍니다.

	FIntPoint EmptyPos;

	// 2. 인벤토리 공간 검사!
	if (InvenSubsys->FindEmptySlot(TempInstance->GetItemSize(), EmptyPos))
	{
		// [성공] 빈자리가 있음!
		InvenSubsys->Items.Add(TempInstance);
		InvenSubsys->AddItemToGrid(TempInstance, EmptyPos);
		InvenSubsys->OnInventoryUpdated.Broadcast();

		UE_LOG(LogTemp, Warning, TEXT("아이템 획득: %s"), *ItemData->ItemName.ToString());

		// TODO: 플레이어 몸으로 빨려 들어가는 연출 추가 (Tick에서 처리)
		Destroy(); // 지금은 즉시 파괴
	}
	else
	{
		// [실패] 인벤토리 꽉 참!
		UE_LOG(LogTemp, Warning, TEXT("인벤토리가 가득 차서 먹을 수 없습니다!"));

		// 가비지 컬렉터가 메모리에서 지우도록 표시 (메모리 릭 방지)
		TempInstance->MarkAsGarbage();

		// TODO: 살짝 통통 튀는 애니메이션 실행 (물리 켜기 등)
	}
}

#include "Object/R1ItemActor.h"
#include "R1LogChannels.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/TextBlock.h"
#include "Components/BoxComponent.h"

#include "Character/R1Player.h"
#include "Item/R1ItemInstance.h"
#include "Item/R1InventorySubsystem.h"
#include "Data/R1ItemAssetData.h"
#include "Item/R1ItemTooltip.h"

#include "Player/R1PlayerController.h"
#include "NiagaraComponent.h"
#include "Library/R1ItemFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "System/R1LocalizationSubsystem.h"

const FName AR1ItemActor::RarityColorParamName = FName("User.RarityColor");
const FName AR1ItemActor::IsLegendaryParamName = FName("User.IsLegendary");

AR1ItemActor::AR1ItemActor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	RootComponent = BoxComp;

	// 임시 기본 크기 (나중에 InitItem에서 바뀜)
	BoxComp->SetBoxExtent(FVector(32.0f, 32.0f, 32.0f));

	BoxComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BoxComp->SetCollisionProfileName(TEXT("PhysicsActor"));
	BoxComp->SetSimulatePhysics(true);
	BoxComp->SetEnableGravity(true);
	BoxComp->SetGenerateOverlapEvents(true);

	TooltipWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("TooltipWidget"));
	TooltipWidget->SetupAttachment(RootComponent);
	TooltipWidget->SetWidgetSpace(EWidgetSpace::Screen); // 화면에 항상 똑바로 보이게
	TooltipWidget->SetDrawAtDesiredSize(true);
	TooltipWidget->SetVisibility(false);


	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 메시는 충돌 끄기

	HaloEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("HaloEffect"));
	HaloEffect->SetupAttachment(MeshComp); // 메시 바로 아래 붙여서 메시 중앙 바닥에 위치하게 함

	HaloEffect->SetAbsolute(false, false, true);
	HaloEffect->SetAutoActivate(false);
	Tags.Add(FName("Item"));
}

void AR1ItemActor::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UR1LocalizationSubsystem* LocSub = GI->GetSubsystem<UR1LocalizationSubsystem>())
			{
				LocSub->OnLanguageChanged.AddUObject(this, &AR1ItemActor::RefreshLocalization);
			}
		}
	}
}

void AR1ItemActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UR1LocalizationSubsystem* LocSub = GI->GetSubsystem<UR1LocalizationSubsystem>())
			{
				LocSub->OnLanguageChanged.RemoveAll(this);
			}
		}
	}
	Super::EndPlay(EndPlayReason);
}

void AR1ItemActor::RefreshLocalization()
{
	UpdateTooltipUI();
}

void AR1ItemActor::Highlight()
{
	if (MeshComp)
	{
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
		MeshComp->SetCustomDepthStencilValue(249);
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

	if (AR1Player* PlayerCharacter = Cast<AR1Player>(Interactor->GetPawn()))
	{
		OnLootAttempted(PlayerCharacter);
	}
}

UPrimitiveComponent* AR1ItemActor::GetInteractTrigger()
{
	return BoxComp;
}

void AR1ItemActor::InitItem(UR1ItemAssetData* InItemData, EItemRarity InRarity, int32 InCount)
{
	ItemData = InItemData;
	ItemRarity = InRarity;
	ItemCount = InCount;

	if (ItemData && TooltipWidget)
	{
		if (ItemData->ItemMesh && MeshComp)
		{
			MeshComp->SetStaticMesh(ItemData->ItemMesh);
			MeshComp->SetRenderCustomDepth(true);
			MeshComp->SetCustomDepthStencilValue(249);

			FVector MinBounds, MaxBounds;
			MeshComp->GetLocalBounds(MinBounds, MaxBounds);

			// Extent는 전체 길이의 절반 값입니다.
			FVector MeshExtent = (MaxBounds - MinBounds) * 0.5f;

			// 💡 플레이어가 클릭하거나 밟기 편하도록 최소 크기(하한선) 보장
			MeshExtent.X = FMath::Max(MeshExtent.X, 50.0f);
			MeshExtent.Y = FMath::Max(MeshExtent.Y, 50.0f);
			MeshExtent.Z = FMath::Max(MeshExtent.Z, 50.0f);

			BoxComp->SetBoxExtent(MeshExtent);
		}
	}
	else
	{
		UE_LOG(LogR1, Error, TEXT("ItemActor: 전달받은 ItemData가 Null입니다!"));
	}

	if (HaloEffect)
	{
		FLinearColor TargetColor;
		float Intensity = 0.5f; // 기본 밝기

		TargetColor = UR1ItemFunctionLibrary::GetRarityColor(ItemRarity).GetSpecifiedColor();

		// 밝기(Intensity) 적용
		TargetColor *= Intensity;
		TargetColor.A = 0.3f; // 알파값 고정

		HaloEffect->SetVariableLinearColor(TEXT("User.RarityColor"), TargetColor);
	}

	UpdateTooltipUI();
}

void AR1ItemActor::OnLootAttempted(AR1Player* Looter)
{
	if (!Looter || !ItemData) return;

	UR1InventorySubsystem* InvenSubsys = GetWorld()->GetSubsystem<UR1InventorySubsystem>();
	if (InvenSubsys && InvenSubsys->AddItemAt(ItemData, ItemRarity,ItemCount, FIntPoint(-1, -1)))
	{
		if (ItemData->WorldLootingSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ItemData->WorldLootingSound, GetActorLocation());
		}

		UE_LOG(LogR1, Warning, TEXT("아이템 획득: %s"), *ItemData->ItemName.ToString());
		Destroy();
	}
	else
	{
		UE_LOG(LogR1, Warning, TEXT("인벤토리가 가득 차서 먹을 수 없습니다!"));
	}
}

void AR1ItemActor::UpdateTooltipUI()
{
	if (!TooltipWidget) return;

	if (!TooltipWidget->GetUserWidgetObject())
	{
		TooltipWidget->InitWidget();
	}

	if (UR1ItemTooltip* Tooltip = Cast<UR1ItemTooltip>(TooltipWidget->GetUserWidgetObject()))
	{
		if (ItemData)
		{
			UWorld* World = GetWorld();
			UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
			UR1LocalizationSubsystem* LocSub = GI ? GI->GetSubsystem<UR1LocalizationSubsystem>() : nullptr;
			FName NameKey = !ItemData->LocalizationKey.IsNone() ? ItemData->LocalizationKey : ItemData->ItemName;
			FText DisplayName = (LocSub && !NameKey.IsNone())
				? LocSub->GetText(NameKey)
				: FText::FromName(ItemData->ItemName);

			Tooltip->SetItemInfo(
				DisplayName,
				ItemRarity,
				ItemCount,
				ItemData->ItemType,
				ItemData->BaseValue,
				false
			);
		}
	}
}

void AR1ItemActor::PopEffect()
{
	BoxComp->SetNotifyRigidBodyCollision(true);
	BoxComp->OnComponentHit.AddDynamic(this, &AR1ItemActor::OnBoxHit);

	float VerticalPower = FMath::RandRange(400.0f, 600.0f);
	FVector PopImpulse = FVector(0.f, 0.f, VerticalPower);
	BoxComp->AddImpulse(PopImpulse, NAME_None, true);
}

void AR1ItemActor::DisablePhysicsAndSetOverlap()
{
	BoxComp->SetSimulatePhysics(false);
	BoxComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	BoxComp->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);
	BoxComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AR1ItemActor::OnBoxHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherComp)
	{
		BoxComp->SetNotifyRigidBodyCollision(false);
		BoxComp->OnComponentHit.RemoveDynamic(this, &AR1ItemActor::OnBoxHit);

		BoxComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
		BoxComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

		if (HaloEffect)
		{
			HaloEffect->Activate(true);
		}

		if (ItemData && ItemData->WorldDropSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ItemData->WorldDropSound, GetActorLocation());
		}

		DisablePhysicsAndSetOverlap();
	}
}

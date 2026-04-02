#include "Object/R1GoldActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Item/R1InventorySubsystem.h" // 서브시스템 인클루드
#include "Player/R1PlayerController.h"
#include "Item/R1ItemTooltip.h"
#include "Components/WidgetComponent.h"

// Sets default values
AR1GoldActor::AR1GoldActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	// 🌟 1. 물리 엔진이 적용될 구체 콜리전을 루트로 설정
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SetRootComponent(SphereComp);
	SphereComp->SetSphereRadius(50.0f); // 적당한 크기
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SphereComp->SetCollisionProfileName(TEXT("PhysicsActor"));
	SphereComp->SetSimulatePhysics(true);
	SphereComp->SetEnableGravity(true);
	SphereComp->SetGenerateOverlapEvents(true);

	// 마우스 클릭 감지를 위해 Visibility 채널 블록

	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AR1GoldActor::OnSphereOverlap);

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

// Called when the game starts or when spawned
void AR1GoldActor::BeginPlay()
{
	Super::BeginPlay();

	UpdateTooltipUI();

	PopEffect();
}

void AR1GoldActor::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this)
	{
		// 겹친 대상이 Pawn(캐릭터)인지 확인합니다.
		if (APawn* PlayerPawn = Cast<APawn>(OtherActor))
		{
			// 몬스터가 밟았을 때 먹어지는 것을 방지하기 위해, 플레이어가 조종 중인지 체크합니다.
			if (PlayerPawn->IsPlayerControlled())
			{
				APlayerController* PC = GetWorld()->GetFirstPlayerController();
				if (PC)
				{
					AR1PlayerController* R1PC = Cast<AR1PlayerController>(PC);
					Interact_Implementation(R1PC);
				}
			}
		}
	}
}

void AR1GoldActor::DisablePhysicsAndSetOverlap()
{
	// 🌟 물리가 다른 캐릭터의 이동을 방해하지 않도록 QueryOnly로 변경
	SphereComp->SetSimulatePhysics(false);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

UPrimitiveComponent* AR1GoldActor::GetInteractTrigger()
{
	return SphereComp.Get();
}

void AR1GoldActor::PopEffect()
{
	SphereComp->SetNotifyRigidBodyCollision(true);
	SphereComp->OnComponentHit.AddDynamic(this, &AR1GoldActor::OnSphereHit);
	float VerticalPower = FMath::RandRange(400.0f, 600.0f);

	FVector PopImpulse = FVector(0.f, 0.f, VerticalPower);
	SphereComp->AddImpulse(PopImpulse, NAME_None, true);
}

void AR1GoldActor::Highlight()
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

void AR1GoldActor::UnHighlight()
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

void AR1GoldActor::Interact_Implementation(AR1PlayerController* Interactor)
{
	if (!Interactor) return;

	UR1InventorySubsystem* InvenSubsys = GetWorld()->GetSubsystem<UR1InventorySubsystem>();
	if (InvenSubsys)
	{
		InvenSubsys->AddGold(GoldAmount);

		UE_LOG(LogTemp, Log, TEXT("골드 %d 획득! 지갑으로 직행!"), GoldAmount);

		// 🔊 (나중에) 여기서 짤랑거리는 골드 획득 사운드를 재생하면 완벽합니다!
	}

	Destroy();
}
void AR1GoldActor::SetGoldAmount(int32 Amount)
{
	GoldAmount = Amount;

	UpdateTooltipUI();
}
void AR1GoldActor::UpdateTooltipUI()
{
	if (!TooltipWidget)
	{
		return;
	}
	// 위젯 컴포넌트가 가지고 있는 껍데기 UI를 가져옵니다.
	if (!TooltipWidget->GetUserWidgetObject())
	{
		TooltipWidget->InitWidget();
	}
	UUserWidget* UserWidget = TooltipWidget->GetUserWidgetObject();

	if (UserWidget)
	{
		// 🌟 2. 캐스팅 시도 및 디버그 로그 출력
		UR1ItemTooltip* Tooltip = Cast<UR1ItemTooltip>(UserWidget);
		if (Tooltip)
		{
			Tooltip->SetItemInfo(
				FText::FromString(TEXT("Gold")),
				EItemRarity::Common,
				GoldAmount,
				ER1ItemType::Consumable,
				0,
				false
			);
			UE_LOG(LogTemp, Log, TEXT("[GoldActor] 툴팁 데이터 세팅 성공! 금액: %d"), GoldAmount);
		}
		else
		{
			// 🚨 캐스팅 실패! (할당된 위젯이 UR1ItemTooltip을 상속받지 않음)
			UE_LOG(LogTemp, Error, TEXT("[GoldActor] 🚨 툴팁 위젯 캐스팅 실패! 현재 할당된 위젯 클래스: %s"), *UserWidget->GetClass()->GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[GoldActor] 🚨 UserWidgetObject가 NULL입니다. 위젯 클래스가 할당되지 않았습니다."));
	}
}

void AR1GoldActor::OnSphereHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherComp)
	{
		// 2. 더 이상 Hit 이벤트가 연달아 터지지 않도록 즉시 차단
		SphereComp->SetNotifyRigidBodyCollision(false);
		SphereComp->OnComponentHit.RemoveDynamic(this, &AR1GoldActor::OnSphereHit);

		// 🌟 3. 미끄러짐이나 덜덜 떨리는 현상이 없도록 남아있는 관성과 회전력을 박살 냅니다!
		SphereComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
		SphereComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

		// 4. 관성을 죽인 후 안전하게 물리 연산 종료 및 오버랩 전환 (기존 함수 재활용)
		DisablePhysicsAndSetOverlap();
	}
}
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
	SphereComp->SetSphereRadius(25.0f); // 적당한 크기
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SphereComp->SetCollisionProfileName(TEXT("PhysicsActor"));
	SphereComp->SetSimulatePhysics(true);
	SphereComp->SetEnableGravity(true);
	SphereComp->SetGenerateOverlapEvents(true);
	// 마우스 클릭 감지를 위해 Visibility 채널 블록
	SphereComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AR1GoldActor::OnSphereOverlap);


	TooltipWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("TooltipWidget"));
	TooltipWidget->SetupAttachment(RootComponent);
	TooltipWidget->SetWidgetSpace(EWidgetSpace::Screen); // 화면에 항상 똑바로 보이게
	TooltipWidget->SetDrawAtDesiredSize(true);
	TooltipWidget->SetVisibility(false);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 메시는 충돌 끄기


	// 🌟 1. 자동 루팅 구체 생성 및 설정
	PickupSphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphereComp"));
	PickupSphereComp->SetupAttachment(RootComponent);

	// 💡 획득 반경 설정 (150.0f 정도면 쾌적하게 먹어집니다. 좁으면 늘리세요!)
	PickupSphereComp->InitSphereRadius(150.0f);

	// 💡 물리 충돌은 끄고, 오직 겹침(Overlap) 판정만 수행하도록 설정합니다.
	PickupSphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupSphereComp->SetCollisionResponseToAllChannels(ECR_Ignore); // 다른 건 다 무시
	PickupSphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // 플레이어(Pawn)와 겹칠 때만 반응!
	Tags.Add(FName("Item"));
}

// Called when the game starts or when spawned
void AR1GoldActor::BeginPlay()
{
	Super::BeginPlay();

	UpdateTooltipUI();

	PopEffect();

	if (PickupSphereComp)
	{
		PickupSphereComp->OnComponentBeginOverlap.AddDynamic(this, &AR1GoldActor::OnPickupSphereOverlap);
	}
}

void AR1GoldActor::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bCanInteract) return;

	// 플레이어가 닿았다면 즉시 획득!
	if (OtherActor && OtherActor->ActorHasTag(FName("Player")))
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			AR1PlayerController* R1PC = Cast<AR1PlayerController>(PC);
			Interact_Implementation(R1PC);
		}
	}
}

void AR1GoldActor::DisablePhysicsAndSetOverlap()
{
	// 🌟 물리가 다른 캐릭터의 이동을 방해하지 않도록 QueryOnly로 변경
	SphereComp->SetSimulatePhysics(false);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	// 모든 채널을 무시하도록 초기화 후, 필요한 것만 켬
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	
	// 1. 마우스 클릭 감지 유지
	SphereComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	
	// 2. 플레이어(Pawn)가 닿는 것은 감지 (Overlap)
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AR1GoldActor::OnPickupSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this)
	{
		// 겹친 대상이 Pawn(캐릭터)인지 확인합니다.
		if (APawn* PlayerPawn = Cast<APawn>(OtherActor))
		{
			// 몬스터가 밟았을 때 먹어지는 것을 방지하기 위해, 플레이어가 조종 중인지 체크합니다.
			if (PlayerPawn->IsPlayerControlled())
			{
				// 유저님이 완벽하게 짜두신 서브시스템 호출!
				if (UR1InventorySubsystem* InvenSubsys = GetWorld()->GetSubsystem<UR1InventorySubsystem>())
				{
					InvenSubsys->AddGold(GoldAmount);

					UE_LOG(LogTemp, Log, TEXT("자동 획득 성공! %d 골드가 지갑에 추가되었습니다."), GoldAmount);

					// 💡 여기에 UGameplayStatics::PlaySoundAtLocation() 으로 짤랑거리는 사운드를 넣으면 완벽합니다!

					// 획득 완료 후 자신은 파괴
					Destroy();
				}
			}
		}
	}
}

UPrimitiveComponent* AR1GoldActor::GetInteractTrigger()
{
	return bCanInteract ? SphereComp.Get() : nullptr;
}

void AR1GoldActor::PopEffect()
{
	float VerticalPower = FMath::RandRange(400.0f, 600.0f);

	FVector PopImpulse = FVector(0.f, 0.f, VerticalPower);
	SphereComp->AddImpulse(PopImpulse, NAME_None, true);

	// 🌟 2. 공중에서 빙글빙글 돌게 회전력(Torque) 추가
	FVector RandomTorque = FVector(FMath::RandRange(-500.f, 500.f), FMath::RandRange(-500.f, 500.f), FMath::RandRange(-500.f, 500.f));
	SphereComp->AddTorqueInDegrees(RandomTorque, NAME_None, true);

	//// 🌟 3. 스케일 애니메이션 (0에서 1로 커지며 생성)
	//SetActorScale3D(FVector::ZeroVector);

	//float TotalTime = 0.2f;
	//float StartTime = GetWorld()->GetTimeSeconds();

	//GetWorldTimerManager().SetTimer(ScaleTimerHandle, [this, StartTime, TotalTime]()
	//	{
	//		float Alpha = FMath::Clamp((GetWorld()->GetTimeSeconds() - StartTime) / TotalTime, 0.0f, 1.0f);
	//		SetActorScale3D(FVector(Alpha, Alpha, Alpha));
	//		if (Alpha >= 1.0f)
	//		{
	//			GetWorldTimerManager().ClearTimer(ScaleTimerHandle);
	//		}
	//	}, 0.01f, true);

	// 🌟 4. 즉시 획득 방지 (0.5초 뒤에 상호작용 가능하게)
	bCanInteract = false;
	FTimerHandle InteractDelayTimer;
	GetWorldTimerManager().SetTimer(InteractDelayTimer, [this]() { bCanInteract = true; }, 0.5f, false);

	// 🌟 5. 물리 시뮬레이션 종료 타이머 (1.5초 뒤에 바닥에 안착했다고 가정하고 물리 연산 끔)
	GetWorldTimerManager().SetTimer(PhysicsTimerHandle, this, &AR1GoldActor::DisablePhysicsAndSetOverlap, 1.5f, false);
}

void AR1GoldActor::Highlight()
{
	if (!bCanInteract) return;

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
	if (!Interactor || !bCanInteract) return;

	UR1InventorySubsystem* InvenSubsys = GetWorld()->GetSubsystem<UR1InventorySubsystem>();
	if (InvenSubsys)
	{
		// 유저님이 이미 완벽하게 만들어두신 AddGold() 호출
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
				FText::FromString(TEXT("금화")),
				EItemRarity::Legendary,
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
#include "Object/R1BladeWaveProjectile.h"
#include "Character/R1Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h" // SphereComponent 접근을 위해 추가

AR1BladeWaveProjectile::AR1BladeWaveProjectile()
{
	// 1. 부모에서 만든 SphereComponent의 충돌 판정을 완전히 끕니다.
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereComponent->SetGenerateOverlapEvents(false);

	// 2. BoxComponent를 생성하고 설정합니다.
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->SetCollisionProfileName(TEXT("Projectile"));
	BoxComponent->InitBoxExtent(FVector(100.f, 20.f, 20.f)); // 검기의 모양에 맞게 x, y, z 크기 조절

	// 3. BoxComponent를 루트 컴포넌트로 만들고, 기존 SphereComponent를 그 아래에 붙입니다.
	SetRootComponent(BoxComponent);
	SphereComponent->SetupAttachment(BoxComponent);

	// 4. 발사체 이동 컴포넌트가 참조하는(업데이트하는) 대상을 BoxComponent로 변경합니다.
	ProjectileMovement->UpdatedComponent = BoxComponent;
	ProjectileMovement->bRotationFollowsVelocity = false;

	// 5. 회전 컴포넌트 설정 (기본적으로 RootComponent를 회전시킵니다)
	RotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));
	RotatingMovement->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
}

void AR1BladeWaveProjectile::SetChargeScale(float InScale)
{
	SetActorScale3D(FVector(InScale));
}

void AR1BladeWaveProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 부모의 BeginPlay에서 SphereComponent에 이벤트를 달았지만, 
	// 콜리전을 껐으므로 발동하지 않습니다. 대신 BoxComponent에 오버랩 이벤트를 직접 달아줍니다.
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AR1BladeWaveProjectile::OnOverlap);
}

void AR1BladeWaveProjectile::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 베이스와 달리 Destroy하지 않는다 — 관통하며 대상마다 1회만 피해 적용.
	if (OtherActor == nullptr || OtherActor == GetInstigator() || HitActors.Contains(OtherActor))
	{
		return;
	}

	AR1Character* TargetCharacter = Cast<AR1Character>(OtherActor);
	if (TargetCharacter == nullptr || TargetCharacter->GetCreatureState() == ECreatureState::Dead)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (TargetASC == nullptr || DamageSpecHandle.IsValid() == false)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
	if (SourceASC == nullptr)
	{
		return;
	}

	HitActors.Add(OtherActor);
	SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpecHandle.Data.Get(), TargetASC);
}



#include "Character/R1Player.h"
#include "Character/R1Monster.h"

#include "Player/R1PlayerController.h"
#include "Player/R1PlayerState.h"

#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"

#include "AbilitySystem/R1AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/R1PlayerAttributeSet.h"

AR1Player::AR1Player()
{
	// APawn이 컨트롤러의 회전을 따라갈지 여부를 결정
	bUseControllerRotationPitch = false;	
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;	// 캐릭터의 움직임에 따라 바라보는 방향을 동기화
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArm->SetupAttachment(GetCapsuleComponent());
	SpringArm->TargetArmLength = 1400.f;
	SpringArm->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bInheritRoll = false;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 3.f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false;	// 캐릭터가 회전할 때 카메라는 고정

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -88.f), FRotator(0.f, -90.f, 0.f));

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}
void AR1Player::BeginPlay()
{
	Super::BeginPlay();

	AttackRange = AttributeSet->GetAttackRange();
}

void AR1Player::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitAbilitySystem();

}

void AR1Player::InitAbilitySystem()
{
	Super::InitAbilitySystem();

	if (AR1PlayerState* PS = GetPlayerState<AR1PlayerState>())
	{
		AbilitySystemComponent = Cast<UR1AbilitySystemComponent>(PS->GetAbilitySystemComponent());
		AbilitySystemComponent->InitAbilityActorInfo(PS, this);
		

		AttributeSet = PS->GetR1PlayerAttributeSet();
	}
}
// Called every frame
void AR1Player::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AR1Player::HandleGameplayEvent(FGameplayTag EventTag)
{
	Super::HandleGameplayEvent(EventTag);
	//TODO
	AR1PlayerController* PC = Cast<AR1PlayerController>(GetController());
	if (PC)
	{
		PC->HandleGameplayEvent(EventTag);
	}
}

void AR1Player::OnDead(const TObjectPtr<AR1Character> Attacker)
{
	Super::OnDead(Attacker);

	AR1PlayerController* PC = Cast<AR1PlayerController>(GetController());
	if (PC)
	{
		PC->PlayerOnDead();
	}
}


void AR1Player::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Log, TEXT("On Begin Overlap"));
}

void AR1Player::ActivateAbility(FGameplayTag AbilityTag)
{
	AbilitySystemComponent->ActivateAbility(AbilityTag);
}

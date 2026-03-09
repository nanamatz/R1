

#include "Character/R1Player.h"
#include "Character/R1Monster.h"
#include "Player/R1PlayerState.h"

#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"

#include "AbilitySystem/R1AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/PlayerAttributeSet.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"

#include "System/R1GameInstance.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"

#include "UI/PlayerInfo/R1ExpBarWidget.h"
#include "DataTable/CharacterStatsRow.h"
#include "R1GameplayTags.h"
#include "System/R1EquipmentManagerComponent.h"

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

	StimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSource"));
	if (StimuliSource)
	{
		StimuliSource->RegisterForSense(UAISense_Sight::StaticClass());

		StimuliSource->RegisterWithPerceptionSystem();
	}

	EquipmentManagerComponent = CreateDefaultSubobject<UR1EquipmentManagerComponent>(TEXT("EquipmentManagerComponent"));
}
void AR1Player::BeginPlay()
{
	Super::BeginPlay();

	if (UR1GameInstance* R1GameInstance = GetGameInstance<UR1GameInstance>())
	{
		//R1GameInstance->ApplyRespawnSnapshotToPlayer(this);
		InitExpBar();
	}
	
	AttackRange = CommonAttributeSet->GetAttackRange();

}

void AR1Player::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitAbilitySystem();

	InitAttributes();

}

void AR1Player::InitAbilitySystem()
{
	Super::InitAbilitySystem();

	if (AR1PlayerState* PS = GetPlayerState<AR1PlayerState>())
	{
		AbilitySystemComponent = Cast<UR1AbilitySystemComponent>(PS->GetAbilitySystemComponent());
		AbilitySystemComponent->InitAbilityActorInfo(PS, this);
		

		CommonAttributeSet = PS->GetCommonAttributeSet();
	}
}
void AR1Player::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	if (OtherActor && OtherActor->IsA(AR1Monster::StaticClass()))
	{
		TArray<AActor*> OverlappingMobs;
		GetCapsuleComponent()->GetOverlappingActors(OverlappingMobs, AR1Monster::StaticClass());

		if (OverlappingMobs.Num() == 0)
		{
			GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
			UE_LOG(LogTemp, Warning, TEXT("몬스터와 겹침이 완전히 해제되어 Block 상태로 복구되었습니다."));
		}
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
}

void AR1Player::OnDead(const TObjectPtr<AR1Character> Attacker)
{
	Super::OnDead(Attacker);

	if (StimuliSource)
	{
		StimuliSource->UnregisterFromPerceptionSystem(); // 죽으면 레이더에서 사라짐!
	}

	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
}



void AR1Player::ActivateAbility(FGameplayTag AbilityTag)
{
	AbilitySystemComponent->ActivateAbility(AbilityTag);
}

void AR1Player::OnManaChanged(float Ratio)
{
	if (OnMpChanged.IsBound())
	{
		OnMpChanged.Broadcast(Ratio);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("OnMpChanged is NOT bound for actor: %s"), *GetName());
	}
}

void AR1Player::InitExpBar()
{
	AR1PlayerState* PS = GetPlayerState<AR1PlayerState>();
	UPlayerAttributeSet* PlayerAttributeSet = PS->GetPlayerAttributeSet();
	if (PS && PlayerAttributeSet)
	{
		float Exp = PlayerAttributeSet->GetExp();
		float MaxExp = PlayerAttributeSet->GetMaxExp();
		float Ratio = Exp / MaxExp;
		PS->OnExpChanged.Broadcast(Ratio);
	}
}

void AR1Player::InitAttributes()
{
	Super::InitAttributes();

	// 2. 플레이어 전용 스탯(마나, 경험치 등) 세팅
	if (!AbilitySystemComponent || !CharacterStatTable || !PlayerInitStatEffectClass) return;

	FR1CharacterStatsRow* StatData = CharacterStatTable->FindRow<FR1CharacterStatsRow>(CharacterRowName, TEXT("InitPlayerAttributes"));
	if (StatData)
	{
		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(PlayerInitStatEffectClass, 1.0f, Context);

		if (SpecHandle.IsValid())
		{
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(R1GameplayTags::Data_Attribute_MaxMana, StatData->MaxMana);
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(R1GameplayTags::Data_Attribute_ManaRegeneration, StatData->ManaRegeneration);
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(R1GameplayTags::Data_Attribute_MaxExp, StatData->MaxExp);
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(R1GameplayTags::Data_Attribute_Level, StatData->Level);
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(R1GameplayTags::Data_Attribute_Mana, StatData->Mana);

			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}

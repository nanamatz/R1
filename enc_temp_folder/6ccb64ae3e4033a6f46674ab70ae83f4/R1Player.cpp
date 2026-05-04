

#include "Character/R1Player.h"
#include "Character/R1Monster.h"
#include "Player/R1PlayerState.h"

#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/PostProcessComponent.h"
#include "AbilitySystem/R1AbilitySystemComponent.h"

#include "Materials/MaterialInstanceDynamic.h"

#include "AbilitySystem/Attribute/PlayerAttributeSet.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"

#include "System/R1GameInstance.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"

#include "UI/PlayerInfo/R1ExpBarWidget.h"
#include "DataTable/CharacterStatsRow.h"
#include "R1GameplayTags.h"
#include "System/R1EquipmentManagerComponent.h"
#include "Player/R1CameraOcclusionComponent.h"
#include "Components/AudioComponent.h"

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
	SpringArm->bDoCollisionTest = false;

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

	PostProcessComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComp"));
	PostProcessComp->SetupAttachment(RootComponent);

	CameraOcclusionComp = CreateDefaultSubobject<UR1CameraOcclusionComponent>(TEXT("CameraOcclusionComp"));

	HeartbeatAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("HeartbeatAudioComponent"));
	HeartbeatAudioComponent->SetupAttachment(RootComponent);
	HeartbeatAudioComponent->bAutoActivate = false;

}
void AR1Player::BeginPlay()
{
	Super::BeginPlay();

	if (LowHealthMaterial)
	{
		LowHealthMI = UMaterialInstanceDynamic::Create(LowHealthMaterial, this);
		if (LowHealthMI)
		{
			PostProcessComp->AddOrUpdateBlendable(LowHealthMI, 1.0f);
		}
	}

	GetMesh()->HideBoneByName(FName("sword_bottom"), PBO_None);
	GetMesh()->HideBoneByName(FName("shield_inner"), PBO_None);
	
	AttackRange = CommonAttributeSet->GetAttackRange();
}

void AR1Player::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitAbilitySystem();

	InitAttributes();

	InitExpBar();
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

void AR1Player::OnHealthChanged(float Ratio)
{
	// 부모(AR1Character)의 UI 갱신 로직 실행
	Super::OnHealthChanged(Ratio);

	// 플레이어 화면 붉어짐 효과 업데이트!
	UpdateLowHealthEffect(Ratio);
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

	if (HeartbeatAudioComponent)
	{
		HeartbeatAudioComponent->Stop();
	}

	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
}



void AR1Player::ActivateAbility(FGameplayTag AbilityTag)
{
	AbilitySystemComponent->ActivateAbility(AbilityTag);
}

void AR1Player::AddCameraYaw(float YawDelta)
{
	if (SpringArm)
	{
		// 카메라 회전 민감도 (필요에 따라 에디터 변수로 빼서 조절하셔도 됩니다)

		// SpringArm의 Z축(Yaw) 방향으로만 회전값을 더해줍니다.
		SpringArm->AddRelativeRotation(FRotator(0.f, YawDelta * RotationSpeed, 0.f));
	}
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
	if (PlayerAttributeSet)
	{
		float Exp = PlayerAttributeSet->GetExp();
		float MaxExp = PlayerAttributeSet->GetMaxExp();
		float Level = PlayerAttributeSet->GetLevel(); // 🌟 레벨 가져오기
		float Ratio = Exp / MaxExp;

		UE_LOG(LogTemp, Warning, TEXT("====================================="));
		UE_LOG(LogTemp, Warning, TEXT("[Player Init] InitExpBar 완료 - UI 동기화됨!"));
		UE_LOG(LogTemp, Warning, TEXT("[Player Init] 현재 인게임 레벨: %f"), Level);
		UE_LOG(LogTemp, Warning, TEXT("[Player Init] 현재 보유 경험치: %f / %f"), Exp, MaxExp);
		UE_LOG(LogTemp, Warning, TEXT("====================================="));
		PS->OnExpChanged.Broadcast(Ratio);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Player Init] PlayerAttributeSet이 아직도 nullptr입니다!"));
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
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(R1GameplayTags::Data_Attribute_HealthRegeneration, StatData->HealthRegeneration);
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(R1GameplayTags::Data_Attribute_MaxExp, StatData->MaxExp);
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(R1GameplayTags::Data_Attribute_Level, StatData->Level);
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(R1GameplayTags::Data_Attribute_Mana, StatData->Mana);


			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
	AR1PlayerState* PS = GetPlayerState<AR1PlayerState>();
	if (PS)
	{
		PS->ApplyMetaUpgrades();
	}
}

void AR1Player::UpdateLowHealthEffect(float Ratio)
{
	if (!LowHealthMI) return;

	if (Ratio <= 0.5f)
	{
		float NormalizedFactor = 1.0f - (Ratio / 0.5f);

		float TargetIntensity = FMath::Pow(NormalizedFactor, 1.5f);

		TargetIntensity *= 1.5f;

		TargetIntensity = FMath::Clamp(TargetIntensity, 0.0f, 1.0f);

		LowHealthMI->SetScalarParameterValue(TEXT("Intensity"), TargetIntensity);

		// [심장 소리 추가 로직]
		if (HeartbeatSound && HeartbeatAudioComponent)
		{
			if (!HeartbeatAudioComponent->IsPlaying())
			{
				HeartbeatAudioComponent->SetSound(HeartbeatSound);
				HeartbeatAudioComponent->Play();
			}

			// NormalizedFactor가 0(체력 50%) -> 1(체력 0%)로 변함
			// 볼륨: 0.5 ~ 1.2 정도로 조절
			float TargetVolume = FMath::Lerp(0.5f, 1.2f, NormalizedFactor);
			// 피치(속도): 1.0 ~ 1.8 정도로 조절
			float TargetPitch = FMath::Lerp(1.0f, 1.8f, NormalizedFactor);

			HeartbeatAudioComponent->SetVolumeMultiplier(TargetVolume);
			HeartbeatAudioComponent->SetPitchMultiplier(TargetPitch);
		}
	}
	else
	{
		LowHealthMI->SetScalarParameterValue(TEXT("Intensity"), 0.0f);

		if (HeartbeatAudioComponent && HeartbeatAudioComponent->IsPlaying())
		{
			HeartbeatAudioComponent->Stop();
		}
	}
}

void AR1Player::TeleportToRoom(FVector TargetLocation)
{
	// CameraBoom은 AR1Player에 선언된 USpringArmComponent 포인터 이름에 맞게 변경해 주세요.
	if (SpringArm)
	{
		// 1. 현재 카메라 지연(Lag) 설정 상태를 기억해 둡니다.
		bool bWasLagging = SpringArm->bEnableCameraLag;

		// 2. 카메라가 부드럽게 따라오지 못하도록 지연 기능을 강제로 끕니다.
		SpringArm->bEnableCameraLag = false;

		// 3. 플레이어를 순간이동 시킵니다. 
		SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);

		// 4. (안전장치) 텔레포트된 위치로 자식 컴포넌트(카메라)들의 위치를 즉시 갱신합니다.
		SpringArm->UpdateChildTransforms();

		// 5. 카메라 지연 기능을 원래대로 되돌려 놓습니다.
		SpringArm->bEnableCameraLag = bWasLagging;
	}
	else
	{
		// 스프링암이 없다면 그냥 이동만 시킵니다.
		SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

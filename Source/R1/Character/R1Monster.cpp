#include "Character/R1Monster.h"
#include "R1Player.h"

#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "AbilitySystem/Attribute/MonsterAttributeSet.h"
#include "AbilitySystem/R1AbilitySystemComponent.h"

#include "AI/R1AIController.h"
#include "BrainComponent.h"
#include "Animation/R1AnimInstance.h"

#include "UI/R1HpBarWidget.h"
#include "UI/R1HUD.h"
#include "AbilitySystemBlueprintLibrary.h"

#include "Map/DungeonManager.h"
#include "DataTable/CharacterStatsRow.h"
#include "TimerManager.h"
#include "R1GameplayTags.h"
#include "Object/R1GoldActor.h"
#include "Kismet/GameplayStatics.h"


AR1Monster::AR1Monster()
{
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -88.f), FRotator(0.f, -90.f, 0.f));
	GetMesh()->CanCharacterStepUpOn = ECB_No;
	GetCapsuleComponent()->CanCharacterStepUpOn = ECB_No;
	AbilitySystemComponent = CreateDefaultSubobject<UR1AbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	MonsterAttributeSet = CreateDefaultSubobject<UMonsterAttributeSet>(TEXT("MonsterAttributeSet"));
	CoreAttributeSet = CreateDefaultSubobject<UR1AttributeSet>(TEXT("CoreAttributeSet"));

	Tags.Add(FName("Enemy"));
}

void AR1Monster::BeginPlay()
{
	Super::BeginPlay();

	InitAbilitySystem();
	
	InitAttributes();
	
	AR1Monster* Monster = Cast<AR1Monster>(this);

	if (GetMesh())
	{
		// 메시의 0번 슬롯 머티리얼을 조종 가능한 '다이내믹'으로 변환해서 저장합니다.
		DissolveMaterial = GetMesh()->CreateDynamicMaterialInstance(0);
		UE_LOG(LogTemp, Warning, TEXT("%s"), *DissolveMaterial.GetName());
	}
}

void AR1Monster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AR1Monster::InitAbilitySystem()
{
	Super::InitAbilitySystem();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

void AR1Monster::ActivateAbility(FGameplayTag AbilityTag)
{
	AbilitySystemComponent->ActivateAbility(AbilityTag);
}

void AR1Monster::OnDead(const TObjectPtr<AR1Character> Attacker)
{
	Super::OnDead(Attacker);

	if (Attacker == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("오류: Attacker가 NULL입니다! 데미지 GE 컨텍스트에서 Instigator를 세팅하지 않았습니다."));
		return;
	}

	AR1AIController* AIC = Cast<AR1AIController>(GetController());

	if (AIC)
	{
		// 리소스 낭비를 막기 위해 AI 로직(비헤이비어 트리) 중단
		AIC->StopMovement();
		AIC->BrainComponent->StopLogic("Dead");
	}

	if (DeathAnimMontage)
	{
		UR1AnimInstance* AnimInstance = Cast<UR1AnimInstance>(GetMesh()->GetAnimInstance());
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(DeathAnimMontage);
		}
	}

	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	}
	RewardExperience(Attacker);
	DropGold();

	CurrentDissolve = 0.0f;
	GetWorldTimerManager().SetTimer(DissolveDelayTimerHandle, this, &AR1Monster::StartDissolve, 2.0f, false);
}

void AR1Monster::InitializeWithManager(ADungeonManager* InManager)
{
	if (!IsValid(InManager)) return;

	InManager->RegisterMonster(this);

	this->OnDeadDelegate.RemoveDynamic(InManager, &ADungeonManager::UnregisterMonster);
	this->OnDeadDelegate.AddDynamic(InManager, &ADungeonManager::UnregisterMonster);

}

void AR1Monster::StartDissolve()
{
	GetWorldTimerManager().SetTimer(DissolveTimerHandle, this, &AR1Monster::UpdateDissolve, 0.05f, true);
}

void AR1Monster::UpdateDissolve()
{
	// 한 번 실행될 때마다 지워지는 양을 0.05씩(5%) 올립니다.
	CurrentDissolve += DissolveConstant;
	// 리모컨의 버튼을 눌러서, 언리얼에서 만든 "DissolveAmount" 다이얼 수치를 변경합니다.
	if (DissolveMaterial)
	{
		DissolveMaterial->SetScalarParameterValue(FName("DissolveAmount"), CurrentDissolve);
	}

	// 다이얼이 1.0(100%) 이상 올라가서 몬스터가 완전히 투명해졌다면?
	if (CurrentDissolve >= 1.0f)
	{
		GetWorldTimerManager().ClearTimer(DissolveTimerHandle);

		//setactorhiddeninGame(true);

		if (OnReadyToSleep.IsBound())
		{
			OnReadyToSleep.Broadcast(this);
		}
	}
}

void AR1Monster::DropGold()
{
	if (!GoldActorClass)
	{
		UE_LOG(LogTemp, Error, TEXT("몬스터 블루프린트에 GoldActorClass가 할당되지 않았습니다!"));
		return;
	}

	if (FMath::RandRange(0.0f, 1.0f) < GoldDropChance)
	{
		FActorSpawnParameters SpawnParams;
		// 🌟 주변에 겹치는 것이 있어도 무조건 스폰되게 설정
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// 🌟 라인 트레이스를 지우고, 몬스터의 중심부에서 그대로 스폰합니다.
		FVector SpawnLocation = GetActorLocation();
		FRotator SpawnRotation = FRotator::ZeroRotator;

		AR1GoldActor* DroppedGold = GetWorld()->SpawnActor<AR1GoldActor>(GoldActorClass, SpawnLocation, SpawnRotation, SpawnParams);

		if (DroppedGold)
		{
			int32 FinalAmount = FMath::RandRange(MinGoldDrop, MaxGoldDrop);
			DroppedGold->SetGoldAmount(FinalAmount);
		}
	}
}

void AR1Monster::OnDamaged(int32 Damage, TObjectPtr<AR1Character> Attacker)
{
	Super::OnDamaged(Damage, Attacker);

	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
	}
}

void AR1Monster::InitAttributes()
{
	// 1. 부모 함수 호출 (체력, 공격력 세팅)
	Super::InitAttributes();

	// 2. 몬스터 전용 스탯(경험치 드롭량, 어그로 범위) 세팅
	if (!AbilitySystemComponent || !CharacterStatTable || !MonsterInitStatEffectClass) return;

	FR1CharacterStatsRow* StatData = CharacterStatTable->FindRow<FR1CharacterStatsRow>(CharacterRowName, TEXT("InitMonsterAttributes"));
	if (StatData)
	{
		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(MonsterInitStatEffectClass, 1.0f, Context);

		if (SpecHandle.IsValid())
		{
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(R1GameplayTags::Data_Attribute_Xp, StatData->Xp);
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(R1GameplayTags::Data_Attribute_AggroRange, StatData->AggroRange);
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(R1GameplayTags::Data_Attribute_AttackAngle, StatData->AttackAngle);

			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}

void AR1Monster::RewardExperience(const TObjectPtr<class AR1Character> Attacker)
{
	if (!Attacker) return;

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Attacker);
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponent();

	// RewardExpGEClass(XpEffect)가 블루프린트에서 잘 할당되어 있는지 확인 후 적용
	if (TargetASC && SourceASC && XpEffect)
	{
		// GE Spec(명세서) 생성
		FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
		ContextHandle.AddInstigator(this, this); // 경험치의 출처 설정

		FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(XpEffect, 1.0f, ContextHandle);

		// 플레이어에게 경험치 GE 적용
		TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void AR1Monster::WakeUp()
{
	//GetWorldTimerManager().ClearTimer(DissolveDelayTimerHandle);
	//GetWorldTimerManager().ClearTimer(DissolveTimerHandle);
	// 1. 화면에 다시 보이기
	SetActorHiddenInGame(false);

	// 2. 충돌 켜기 (기본 캐릭터 프리셋 기준)
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	// 3. 틱 연산 및 움직임 복구
	SetActorTickEnabled(true);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	// 4. 상태 및 태그 초기화 (부모의 SetCreatureState 활용)
	// 이 함수를 호출하면 내부적으로 "Character.State.Dead" 태그가 알아서 제거됩니다!
	SetCreatureState(ECreatureState::Idle);

	// 5. AI 재가동
	AR1AIController* AIC = Cast<AR1AIController>(GetController());
	if (AIC && AIC->BrainComponent)
	{
		AIC->BrainComponent->RestartLogic();
	}

	// 6. 몬스터 스탯 초기화 (GAS)
	// InitAttributes()를 다시 불러 BaseDamage, MaxHealth 등을 덮어씌웁니다.
	InitAttributes();

	// [중요] InitAttributes()가 MaxHealth는 세팅하지만 현재 Health를 Max로 안 채워줄 수 있습니다.
	// 따라서 현재 체력을 MaxHealth 강제로 채워줍니다.
	if (AbilitySystemComponent && CoreAttributeSet)
	{
		float MaxHp = CoreAttributeSet->GetMaxHealth();
		// 현재 체력을 베이스 값으로 리셋
		AbilitySystemComponent->SetNumericAttributeBase(CoreAttributeSet->GetHealthAttribute(), MaxHp);
	}

	// 7. HP 바 갱신 및 표시
	//if (HpBarComponent)
	//{
	//	HpBarComponent->SetHiddenInGame(false);
	//	RefreshHpBar(1.0f);
	//}

	// 8. 디졸브 원상복구
	CurrentDissolve = 0.0f;
	if (DissolveMaterial)
	{
		DissolveMaterial->SetScalarParameterValue(FName("DissolveAmount"), 0.0f);
	}

	// 진행 중이던 데스 애니메이션 등 취소
	if (GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->StopAllMontages(0.0f);
	}
}

void AR1Monster::GoToSleep()
{
	// 1. 화면에서 완전히 숨기기
	SetActorHiddenInGame(true);

	// 2. 물리 및 충돌 끄기
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 3. 틱 연산 끄기
	SetActorTickEnabled(false);

	// 4. 움직임 정지 (공중에 멈춰있게)
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	// 5. HP 바 위젯 끄기 (OnDead에서 하셨지만 여기서 한 번 더 확실하게)
	//if (HpBarComponent)
	//{
	//	HpBarComponent->SetHiddenInGame(true);
	//}
}

void AR1Monster::Highlight()
{
	Super::Highlight();

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AR1HUD* HUD = Cast<AR1HUD>(PC->GetHUD()))
		{
			HUD->ShowMonsterInfo(this);
		}
	}
}

void AR1Monster::UnHighlight()
{
	Super::UnHighlight();

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AR1HUD* HUD = Cast<AR1HUD>(PC->GetHUD()))
		{
			HUD->HideMonsterInfo();
		}
	}
}

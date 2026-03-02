


#include "Player/R1PlayerController.h"
#include "Character/R1Player.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "System/R1AssetManager.h"
#include "System/R1GameInstance.h"

#include "Data/R1InputData.h"
#include "R1GameplayTags.h"
#include "UI/R1HUD.h"

#include "AbilitySystem/R1AbilitySystemComponent.h"

AR1PlayerController::AR1PlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CacheDestination = FVector::ZeroVector;
	FollowTime = 0.f;

}

void AR1PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (const UR1InputData* InputData = UR1AssetManager::GetAssetByName<UR1InputData>("InputData"))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputData->InputMappingContext, 0);
		}
	}

	R1Player = Cast<AR1Player>(GetCharacter());
	if (R1Player)
	{
		R1Player->OnDeadDelegate.AddDynamic(this, &ThisClass::HandlePlayerDead);
	}

	// 💡 새 레벨이 시작될 때마다 마우스 캡처 깜빡임을 방지하고 GameAndUI 모드로 확정 짓습니다.
	UpdateInputMode(false);
	R1Player->SetCreatureState(ECreatureState::Moving);
}

void AR1PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (const UR1InputData* InputData = UR1AssetManager::GetAssetByName<UR1InputData>("InputData"))
	{
		UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
		if (EnhancedInputComponent == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("SetupInputComponent failed: InputComponent is not UEnhancedInputComponent."));
			return;
		}

		auto ActionMoveTo = InputData->FindInputActionByTag(R1GameplayTags::Input_Action_SetDestination);
		if (ActionMoveTo == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("SetupInputComponent failed: ActionMoveTo is null."));
			return;
		}

		EnhancedInputComponent->BindAction(ActionMoveTo, ETriggerEvent::Started, this, &ThisClass::OnInputStarted);
		EnhancedInputComponent->BindAction(ActionMoveTo, ETriggerEvent::Triggered, this, &ThisClass::OnSetDestinationTriggered);
		EnhancedInputComponent->BindAction(ActionMoveTo, ETriggerEvent::Completed, this, &ThisClass::OnSetDestinationReleased);
		EnhancedInputComponent->BindAction(ActionMoveTo, ETriggerEvent::Canceled, this, &ThisClass::OnSetDestinationReleased);

		auto ActionInventoryToggle = InputData->FindInputActionByTag(R1GameplayTags::Input_Action_Inventory);

		if (ActionInventoryToggle == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("SetupInputComponent failed: ActionInventroyToggle is null."));
			return;
		}
		
		EnhancedInputComponent->BindAction(ActionInventoryToggle, ETriggerEvent::Started, this, &ThisClass::OnInventoryToggle);

		auto ActionGameMenuToggle = InputData->FindInputActionByTag(R1GameplayTags::Input_Action_GameMenu);

		if (ActionGameMenuToggle == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("SetupInputComponent failed: ActionGameMenuToggle is null."));
			return;
		}

		EnhancedInputComponent->BindAction(ActionGameMenuToggle, ETriggerEvent::Started, this, &ThisClass::OnGameMenuToggle);


		auto ActionQSkill = InputData->FindInputActionByTag(R1GameplayTags::Input_Action_SkillQ);

		if (ActionQSkill == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("SetupInputComponent failed: ActioQSkill is null."));
			return;
		}

		EnhancedInputComponent->BindAction(ActionQSkill, ETriggerEvent::Triggered, this, &ThisClass::OnQSkill);

		auto ActionWSkill = InputData->FindInputActionByTag(R1GameplayTags::Input_Action_SkillW);

		if (ActionWSkill == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("SetupInputComponent failed: ActioWSkill is null."));
			return;
		}

		EnhancedInputComponent->BindAction(ActionWSkill, ETriggerEvent::Triggered, this, &ThisClass::OnWSkill);

		auto ActionESkill = InputData->FindInputActionByTag(R1GameplayTags::Input_Action_SkillE);

		if (ActionESkill == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("SetupInputComponent failed: ActioESkill is null."));
			return;
		}

		EnhancedInputComponent->BindAction(ActionESkill, ETriggerEvent::Triggered, this, &ThisClass::OnESkill);

		auto ActionRSkill = InputData->FindInputActionByTag(R1GameplayTags::Input_Action_SkillR);

		if (ActionRSkill == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("SetupInputComponent failed: ActioRSkill is null."));
			return;
		}

		EnhancedInputComponent->BindAction(ActionRSkill, ETriggerEvent::Triggered, this, &ThisClass::OnRSkill);

	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AR1PlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	TickCursorTrace();

	if (R1Player == nullptr)
	{
		R1Player = Cast<AR1Player>(GetCharacter());
		if (R1Player == nullptr)
		{
			return;
		}
	}

	if (R1Player->GetCreatureState() != ECreatureState::Dead)
	{
		ChaseTargetAndAttack();
	}
}

void AR1PlayerController::OnInputStarted()
{
	StopMovement();
	bMousePressed = true;
	TargetActor = HighlightActor;
}

void AR1PlayerController::OnSetDestinationTriggered()
{
	if (R1Player && R1Player->GetCreatureState() == ECreatureState::Casting)
	{
		return;
	}

	if (TargetActor)
	{
		return;
	}

	FollowTime += GetWorld()->GetDeltaSeconds();

	FHitResult Hit;
	bool bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);

	if (bHitSuccessful)
	{
		CacheDestination = Hit.Location;
	}

	if (R1Player)
	{

		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CacheDestination);
	}
}

void AR1PlayerController::OnSetDestinationReleased()
{
	bMousePressed = false;

	if (R1Player && R1Player->GetCreatureState() == ECreatureState::Casting)
	{
		return;
	}

	if (FollowTime <= ShortPressThreshold)
	{
		if (TargetActor == nullptr)
		{
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CacheDestination);
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, CacheDestination, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
		}
	}

	FollowTime = 0.f;
}

void AR1PlayerController::TickCursorTrace()
{
	if (bMousePressed)
	{
		return;
	}

	FHitResult OutCursorHit;
	if (GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, OutCursorHit) == false )
	{
		return;
	}

	/*UE_LOG(LogTemp,Warning,TEXT("%s"),*OutCursorHit.GetActor()->GetName());*/

	SwitchCursorType(OutCursorHit);

	AR1Character* LocalHighlightActor = Cast<AR1Character>(OutCursorHit.GetActor());
	if (LocalHighlightActor == nullptr)
	{
		if (HighlightActor)
		{
			HighlightActor->UnHighlight();
		}
	}
	else
	{
		if (HighlightActor)
		{
			if (HighlightActor != LocalHighlightActor)
			{
				HighlightActor->UnHighlight();
				LocalHighlightActor->Highlight();
			}
		}
		else
		{
			LocalHighlightActor->Highlight();
		}
	}
	HighlightActor = LocalHighlightActor;
}

void AR1PlayerController::ChaseTargetAndAttack()
{
	if (R1Player == nullptr || TargetActor == nullptr)
	{
		return;
	}

	if (R1Player && R1Player->GetCreatureState() == ECreatureState::Casting)
	{
		return;
	}



	if (TargetActor && bMousePressed)
	{
		FVector Direction = TargetActor->GetActorLocation() - R1Player->GetActorLocation();

		TargetAttackActor = TargetActor;

		FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(R1Player->GetActorLocation(), TargetAttackActor->GetActorLocation());
		R1Player->SetActorRotation(Rotation);

		if (Direction.Length() < R1Player->AttackRange && TargetAttackActor)
		{
			R1Player->ActivateAbility(R1GameplayTags::Ability_Attack);
			
			UE_LOG(LogTemp,Warning,TEXT("Attack Count : %d"),++AttackCount);
			
			TargetActor = HighlightActor;
		}
		else
		{
			//too far you should move
			CacheDestination = TargetActor->GetActorLocation();
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CacheDestination);
		}
	}
	
	
}

void AR1PlayerController::SwitchCursorType(FHitResult& OutHit)
{
	AActor* CurrentActorType = OutHit.GetActor();
	if (CurrentActorType == nullptr)
	{
		CurrentMouseCursor = EMouseCursor::Default;
		return;
	}

	if(CurrentActorType->ActorHasTag(FName("Enemy")))
	{
		CurrentMouseCursor = EMouseCursor::Crosshairs;
	}
	else if(CurrentActorType->ActorHasTag(FName("Interactable")))
	{
		CurrentMouseCursor = EMouseCursor::Hand;
	}
	else
	{
		CurrentMouseCursor = EMouseCursor::Default;
	}
	
}

void AR1PlayerController::PlayerOnDead()
{
	if (R1Player == nullptr)
	{
		return;
	}

	if (R1Player->GetCreatureState() == ECreatureState::Dead)
	{
		if (const UR1InputData* InputData = UR1AssetManager::GetAssetByName<UR1InputData>("InputData"))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
			{
				if (Subsystem->HasMappingContext(InputData->InputMappingContext))
				{
					Subsystem->RemoveMappingContext(InputData->InputMappingContext);
				}
			}
		}
		UpdateInputMode(R1Player->GetCreatureState() == ECreatureState::Dead);

		AR1HUD* MyR1HUD = GetHUD<AR1HUD>();
		if (MyR1HUD)
		{
			MyR1HUD->UpdateGameOverUI();
		}
	}
}

void AR1PlayerController::HandlePlayerDead(AR1Character* DeadCharacter, AR1Character* Attacker)
{
	(void)Attacker;

	if (DeadCharacter == R1Player)
	{
		PlayerOnDead();
	}
}

void AR1PlayerController::ResetMovementState()
{
	// 1. 현재 AI(네비게이션) 시스템이 실행 중인 경로 탐색 및 이동을 즉시 정지시킵니다!
	StopMovement();

	// 2. 마우스를 누르고 있던 상태나, 마우스 홀드 시간도 리셋합니다.
	bMousePressed = false;
	FollowTime = 0.f;

	// 3. 클릭해두었던 타겟(문이나 몬스터)을 비워줍니다.
	TargetActor = nullptr;
	TargetAttackActor = nullptr;

	// 4. CacheDestination을 내 현재 위치로 덮어씌워서 완벽하게 초기화합니다.
	// (0, 0, 0으로 하면 혹시나 다시 이동 명령이 들어갔을 때 맵 중앙으로 뛸 수 있으니 내 위치로 하는 게 안전합니다)
	if (R1Player)
	{
		CacheDestination = R1Player->GetActorLocation();
	}
}

void AR1PlayerController::OnInventoryToggle()
{
	AR1HUD* MyR1HUD = GetHUD<AR1HUD>();
	if (MyR1HUD)
	{
		MyR1HUD->ToggleInventory();
	}
}

void AR1PlayerController::OnQSkill()
{
	if (!R1Player) return;

	UAbilitySystemComponent* ASC = R1Player->GetAbilitySystemComponent();
	if (!ASC) return;

	// 1. 현재 장착된 장비나 스킬 매니저로부터 Q슬롯에 해당하는 'GA 클래스'를 가져옵니다.
	// (이 함수는 질문자님의 장비 시스템 구조에 맞게 구현하시면 됩니다)
	TSubclassOf<UGameplayAbility> QSkillClass = R1Player->GetEquippedSkillClass(EInputSlot::Q);

	if (QSkillClass)
	{
		// 2. 클래스로 직접 실행 시도!
		bool bSuccess = ASC->TryActivateAbilityByClass(QSkillClass);

		if (bSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("✅ [컨트롤러] Q 스킬 직접 실행 성공!"));
		}
		else
		{
			// 실행 실패 시 (쿨타임, 마나 부족, 혹은 CanActivateAbility에서 false 반환)
			UE_LOG(LogTemp, Error, TEXT("🚨 [컨트롤러] Q 스킬 조건 미달로 실행 실패 (마나/사거리/쿨타임 등)"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ℹ️ [컨트롤러] Q슬롯에 장착된 스킬이 없습니다."));
	}
}

void AR1PlayerController::OnWSkill()
{
	UR1AbilitySystemComponent* ASC = Cast<UR1AbilitySystemComponent>(R1Player->GetAbilitySystemComponent());

	FGameplayEventData PayloadData;
	ASC->HandleGameplayEvent(R1GameplayTags::Input_Action_SkillW, &PayloadData);
}

void AR1PlayerController::OnESkill()
{
	UR1AbilitySystemComponent* ASC = Cast<UR1AbilitySystemComponent>(R1Player->GetAbilitySystemComponent());

	FGameplayEventData PayloadData;
	ASC->HandleGameplayEvent(R1GameplayTags::Input_Action_SkillE, &PayloadData);
}


void AR1PlayerController::OnRSkill()
{
	UR1AbilitySystemComponent* ASC = Cast<UR1AbilitySystemComponent>(R1Player->GetAbilitySystemComponent());

	FGameplayEventData PayloadData;
	ASC->HandleGameplayEvent(R1GameplayTags::Input_Action_SkillR, &PayloadData);
}

void AR1PlayerController::OnGameMenuToggle()
{
	bool bIsPaused = IsPaused();
	if (bIsPaused)
	{
		AR1HUD* MyR1HUD = GetHUD<AR1HUD>();

		if (MyR1HUD)
		{
			MyR1HUD->ToggleGameMenu();
		}

		SetPause(false);
	}
	else
	{
		AR1HUD* MyR1HUD = GetHUD<AR1HUD>();

		if (MyR1HUD)
		{
			MyR1HUD->ToggleGameMenu();
		}

		SetPause(true);
	}

}

void AR1PlayerController::UpdateInputMode(bool bShouldUIOnly)
{
	if (bShouldUIOnly)
	{
		FInputModeUIOnly InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
	else
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
}

void AR1PlayerController::HandleGameplayEvent(FGameplayTag EventTag)
{
	//TODO
}

void AR1PlayerController::RespawnInLevel(FName LevelName)
{
	if (LevelName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("RespawnInLevel failed: LevelName is None."));
		return;
	}

	if (UR1GameInstance* R1GameInstance = GetGameInstance<UR1GameInstance>())
	{
		R1GameInstance->SaveRespawnSnapshotFromPlayer(R1Player);
	}

	UGameplayStatics::OpenLevel(this, LevelName);
}

void AR1PlayerController::RespawnCurrentLevel()
{
	const FName CurrentLevelName = FName(*UGameplayStatics::GetCurrentLevelName(this, true));
	RespawnInLevel(CurrentLevelName);
}


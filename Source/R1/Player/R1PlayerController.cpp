#include "Player/R1PlayerController.h"

#include "AbilitySystem/R1AbilitySystemComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

#include "Character/R1Player.h"
#include "Character/R1Monster.h"

#include "Data/R1InputData.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "GameFramework/CharacterMovementComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "Item/R1InventorySubsystem.h"
#include "Item/R1ItemInstance.h"
#include "Interface/R1InteractionInterface.h"

#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Navigation/PathFollowingComponent.h"

#include "Object/R1ItemActor.h"
#include "R1GameplayTags.h"

#include "System/R1AssetManager.h"
#include "System/R1GameInstance.h"
#include "System/R1EquipmentManagerComponent.h"

#include "UI/R1HUD.h"

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

		EnhancedInputComponent->BindAction(ActionQSkill, ETriggerEvent::Started, this, &ThisClass::OnQSkill);

		auto ActionWSkill = InputData->FindInputActionByTag(R1GameplayTags::Input_Action_SkillW);

		if (ActionWSkill == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("SetupInputComponent failed: ActioWSkill is null."));
			return;
		}

		EnhancedInputComponent->BindAction(ActionWSkill, ETriggerEvent::Started, this, &ThisClass::OnWSkill);

		auto ActionESkill = InputData->FindInputActionByTag(R1GameplayTags::Input_Action_SkillE);

		if (ActionESkill == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("SetupInputComponent failed: ActioESkill is null."));
			return;
		}

		EnhancedInputComponent->BindAction(ActionESkill, ETriggerEvent::Started, this, &ThisClass::OnESkill);

		auto ActionRSkill = InputData->FindInputActionByTag(R1GameplayTags::Input_Action_SkillR);

		if (ActionRSkill == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("SetupInputComponent failed: ActioRSkill is null."));
			return;
		}

		EnhancedInputComponent->BindAction(ActionRSkill, ETriggerEvent::Started, this, &ThisClass::OnRSkill);

		auto ActionLookClick = InputData->FindInputActionByTag(R1GameplayTags::Input_Action_LookClick);

		if (ActionLookClick == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("SetupInputComponent failed: ActionLookClick is null."));
			return;
		}

		EnhancedInputComponent->BindAction(ActionLookClick, ETriggerEvent::Started, this, &ThisClass::OnLookClickStarted);
		EnhancedInputComponent->BindAction(ActionLookClick, ETriggerEvent::Completed, this, &ThisClass::OnLookClickReleased);
		EnhancedInputComponent->BindAction(ActionLookClick, ETriggerEvent::Canceled, this, &ThisClass::OnLookClickReleased);

		auto ActionLookMouse = InputData->FindInputActionByTag(R1GameplayTags::Input_Action_LookMouse);

		if (ActionLookMouse == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("SetupInputComponent failed: ActionLookMouse is null."));
			return;
		}
		EnhancedInputComponent->BindAction(ActionLookMouse, ETriggerEvent::Triggered, this, &ThisClass::OnLookMouse);
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

		// 🌟 [추가된 핵심 로직] 캐릭터의 실제 속도를 감지하여 상태를 자동으로 되돌립니다!
		if (R1Player->GetCreatureState() != ECreatureState::Casting)
		{
			// SizeSquared()는 벡터의 길이를 제곱한 값으로, 미세한 떨림을 무시하기 위해 10.0f 정도의 여유를 줍니다.
			if (R1Player->GetVelocity().SizeSquared() <= 10.0f)
			{
				// 목적지에 도착해서 속도가 0이 되면 Idle로 복귀!
				if (R1Player->GetCreatureState() == ECreatureState::Moving)
				{
					R1Player->SetCreatureState(ECreatureState::Idle);
				}
			}
			else
			{
				// (혹시 모를 외부 요인으로 밀려나거나 움직일 때를 대비한 방어 코드)
				if (R1Player->GetCreatureState() == ECreatureState::Idle)
				{
					R1Player->SetCreatureState(ECreatureState::Moving);
				}
			}
		}
	}
}

void AR1PlayerController::OnInputStarted()
{
	//StopMovement();
	bMousePressed = true;
	TargetActor = HighlightActor;
}

void AR1PlayerController::OnSetDestinationTriggered()
{
	if (!bMousePressed) return;
	if (R1Player && R1Player->GetCreatureState() == ECreatureState::Casting) return;
	if (TargetActor) return;

	FollowTime += GetWorld()->GetDeltaSeconds();

	FHitResult Hit;
	bool bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_GameTraceChannel2, false, Hit);

	if (bHitSuccessful)
	{
		// 마우스를 조금 움직였다고 매 프레임 재계산하는 것을 막아줍니다. (50 유닛 기준)
		if (FVector::Dist(CacheDestination, Hit.Location) > 50.0f)
		{
			CacheDestination = Hit.Location; // 목적지 갱신

			if (R1Player && R1Player->GetCreatureState() != ECreatureState::Moving)
			{
				R1Player->SetCreatureState(ECreatureState::Moving);
			}

			// 이제 거리가 50 이상 변했을 때만 이동 명령을 내립니다!
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CacheDestination);
		}
	}
}

void AR1PlayerController::OnSetDestinationReleased()
{
	if (!bMousePressed) return;

	bMousePressed = false;

	if (R1Player && R1Player->GetCreatureState() == ECreatureState::Casting)
	{
		return;
	}

	AR1HUD* MyR1HUD = GetHUD<AR1HUD>();
	
	if (MyR1HUD && MyR1HUD->bIsShopUIVisible == true)
	{
		return;
	}


	if (FollowTime <= ShortPressThreshold)
	{
		if (TargetActor == nullptr)
		{
			if (R1Player->GetCreatureState() != ECreatureState::Moving)
			{
				R1Player->SetCreatureState(ECreatureState::Moving);
			}
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
	if (GetHitResultUnderCursor(ECollisionChannel::ECC_GameTraceChannel2, false, OutCursorHit) == false )
	{
		return;
	}

	/*UE_LOG(LogTemp,Warning,TEXT("%s"),*OutCursorHit.GetActor()->GetName());*/

	SwitchCursorType(OutCursorHit);

	AActor* LocalHighlightActor = OutCursorHit.GetActor();
	IR1HighlightInterface* HighlightableActor = Cast<IR1HighlightInterface>(LocalHighlightActor);

	if (HighlightableActor)
	{
		// 마우스 아래에 몬스터나 아이템이 있다!
		if (HighlightActor != LocalHighlightActor)
		{
			// 예전 타겟은 불 끄기
			if (IR1HighlightInterface* OldHighlight = Cast<IR1HighlightInterface>(HighlightActor))
			{
				OldHighlight->UnHighlight();
			}

			// 새 타겟 불 켜기
			HighlightableActor->Highlight();

			// 🌟 타겟 갱신 (진짜 타겟만 들어옵니다)
			HighlightActor = LocalHighlightActor;
		}
	}
	else
	{
		// 🌟 2. 마우스 아래에 맨땅(Floor)이나 벽이 있다!
		if (HighlightActor)
		{
			// 불 끄고
			if (IR1HighlightInterface* OldHighlight = Cast<IR1HighlightInterface>(HighlightActor))
			{
				OldHighlight->UnHighlight();
			}
			// 🌟 타겟을 완전히 비워버립니다! (이제 땅을 클릭해도 TargetActor에 안 들어갑니다)
			HighlightActor = nullptr;
		}
	}
}

void AR1PlayerController::ChaseTargetAndAttack()
{
	if (R1Player == nullptr || TargetActor == nullptr) return;
	if (R1Player && R1Player->GetCreatureState() == ECreatureState::Casting) return;
	UE_LOG(LogTemp, Warning, TEXT("TargetActor: %s"), TargetActor ? *TargetActor->GetName() : TEXT("None"));

	TargetAttackActor = Cast<AR1Monster>(TargetActor);
	if (TargetAttackActor)
	{
		FVector Direction = TargetAttackActor->GetActorLocation() - R1Player->GetActorLocation();
		FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(R1Player->GetActorLocation(), TargetAttackActor->GetActorLocation());
		R1Player->SetActorRotation(Rotation);

		if (Direction.Length() < R1Player->AttackRange)
		{
			StopMovement();
			if (TargetAttackActor->GetCreatureState() == ECreatureState::Dead) return;

			R1Player->ActivateAbility(R1GameplayTags::Ability_Attack);
			//TargetAttackActor = nullptr;
		}
		else
		{
			CacheDestination = TargetAttackActor->GetActorLocation();
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CacheDestination);
		}
	}
	else if (IR1InteractionInterface* InteractableTarget = Cast<IR1InteractionInterface>(TargetActor))
	{
		bool bIsInRange = false;
		UPrimitiveComponent* TriggerComp = InteractableTarget->GetInteractTrigger();
		if (TriggerComp) bIsInRange = TriggerComp->IsOverlappingActor(R1Player);

		if (bIsInRange)
		{
			IR1InteractionInterface::Execute_Interact(TargetActor, this);
			//ResetMovementState();
		}
		else
		{
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
	if (UPathFollowingComponent* PathComp = FindComponentByClass<UPathFollowingComponent>())
	{
		PathComp->AbortMove(*this, FPathFollowingResultFlags::MovementStop);
	}

	// 2. 캐릭터의 남은 물리적 관성(미끄러짐)까지 완벽하게 제거하여 제자리에 못 박습니다.
	if (R1Player && R1Player->GetCharacterMovement())
	{
		R1Player->GetCharacterMovement()->StopMovementImmediately();
		CacheDestination = R1Player->GetActorLocation(); // 캐시도 현재 위치로 덮어쓰기
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CacheDestination);
	}

	if (HighlightActor)
	{
		if (IR1HighlightInterface* OldHighlight = Cast<IR1HighlightInterface>(HighlightActor))
		{
			OldHighlight->UnHighlight();
		}
	}

	// 3. 마우스 및 타겟 상태 초기화
	bMousePressed = false;
	FollowTime = 0.f;

	TargetActor = nullptr;
	TargetAttackActor = nullptr;
	HighlightActor = nullptr;
}

AR1Character* AR1PlayerController::GetHighlightActor()
{
	if (R1Player)
	{
		return R1Player->CombatTarget;
	}
	return nullptr;
}

void AR1PlayerController::DropItemToWorld(UR1ItemInstance* ItemToDrop, ER1EquipmentSlot FromEquipSlot)
{
	if (!ItemToDrop || !R1Player || !ItemActorClass) return;

	UR1InventorySubsystem* InvenSubsys = GetWorld()->GetSubsystem<UR1InventorySubsystem>();
	if (!InvenSubsys) return;

	FVector SpawnLoc = R1Player->GetActorLocation();

	// 2. 월드에 아이템 액터 스폰!
	FActorSpawnParameters SpawnParams;
	AR1ItemActor* DroppedItem = GetWorld()->SpawnActor<AR1ItemActor>(ItemActorClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	DroppedItem->PopEffect();

	if (DroppedItem)
	{
		// 3. 인스턴스가 들고 있던 정보 그대로 전달
		DroppedItem->InitItem(ItemToDrop->GetItemData(), ItemToDrop->ItemRarity,ItemToDrop->ItemCount);

		if (FromEquipSlot != ER1EquipmentSlot::None)
		{
			// A. 장비창에서 밖으로 바로 던진 경우!
			InvenSubsys->EquippedItems.Remove(FromEquipSlot); // 서브시스템 데이터 삭제

			// GAS(스킬, 스탯) 버프 회수
			if (UR1EquipmentManagerComponent* EquipComp = R1Player->FindComponentByClass<UR1EquipmentManagerComponent>())
			{
				EquipComp->UnEquipItem(FromEquipSlot);
			}
		}
		else
		{
			// B. 인벤토리에서 밖으로 던진 경우!
			InvenSubsys->Items.Remove(ItemToDrop);
			InvenSubsys->RemoveItemFromGrid(ItemToDrop, InvenSubsys->GetItemPosition(ItemToDrop));
		}

		InvenSubsys->OnInventoryUpdated.Broadcast();

		UE_LOG(LogTemp, Warning, TEXT("아이템을 월드에 버렸습니다!"));
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
	if (R1Player && R1Player->GetEquipmentComponent() && R1Player->GetCreatureState() != ECreatureState::Casting)
	{
		R1Player->CombatTarget = Cast<AR1Character>(HighlightActor);
		R1Player->GetEquipmentComponent()->ExecuteSkillSlot(ER1SkillSlot::Q);
	}
}

void AR1PlayerController::OnWSkill()
{
	if (R1Player && R1Player->GetEquipmentComponent() && R1Player->GetCreatureState() != ECreatureState::Casting)
	{
		R1Player->GetEquipmentComponent()->ExecuteSkillSlot(ER1SkillSlot::W);
	}
}

void AR1PlayerController::OnESkill()
{
	if (R1Player && R1Player->GetEquipmentComponent() && R1Player->GetCreatureState() != ECreatureState::Casting)
	{
		R1Player->GetEquipmentComponent()->ExecuteSkillSlot(ER1SkillSlot::E);
	}
}


void AR1PlayerController::OnRSkill()
{
	if (R1Player && R1Player->GetEquipmentComponent() && R1Player->GetCreatureState() != ECreatureState::Casting)
	{
		R1Player->GetEquipmentComponent()->ExecuteSkillSlot(ER1SkillSlot::R);
	}
}


void AR1PlayerController::OnLookClickStarted()
{
	bIsCameraRotating = true;
}

void AR1PlayerController::OnLookClickReleased()
{
	bIsCameraRotating = false;
}

void AR1PlayerController::OnLookMouse(const FInputActionValue& Value)
{
	// 마우스 회전 트리거 버튼을 꾹 누르고 있을 때만 작동합니다.
	if (!bIsCameraRotating || !R1Player) return;

	// X축 마우스 이동량 추출
	float LookValue = Value.Get<float>();

	if (LookValue != 0.0f)
	{
		// 플레이어 캐릭터의 스프링암을 돌립니다.
		R1Player->AddCameraYaw(LookValue);
	}
}

void AR1PlayerController::OnGameMenuToggle()
{
	AR1HUD* MyR1HUD = GetHUD<AR1HUD>();
	if (MyR1HUD)
	{
		MyR1HUD->ToggleGameMenu();
	}

	// 2. 현재 상태 확인 후 반전
	bool bIsPaused = IsPaused();
	SetPause(!bIsPaused); // true면 false로, false면 true로!
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


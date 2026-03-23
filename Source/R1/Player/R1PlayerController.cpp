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
#include "System/R1EquipmentManagerComponent.h"

#include "Item/R1InventorySubsystem.h"
#include "Item/R1ItemInstance.h"
#include "Object/R1ItemActor.h"

#include "Data/R1InputData.h"
#include "R1GameplayTags.h"
#include "UI/R1HUD.h"
#include "Interface/R1InteractionInterface.h"
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
	if (R1Player == nullptr || TargetActor == nullptr)
	{
		return;
	}

	if (R1Player && R1Player->GetCreatureState() == ECreatureState::Casting)
	{
		return;
	}
	
	if(bMousePressed == false)
	{
		return;
	}

	TargetAttackActor = Cast<AR1Character>(TargetActor);
	
	if (TargetAttackActor)
	{
		FVector Direction = TargetAttackActor->GetActorLocation() - R1Player->GetActorLocation();

		FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(
			R1Player->GetActorLocation(), 
			TargetAttackActor->GetActorLocation()
		);
		R1Player->SetActorRotation(Rotation);

		if (Direction.Length() < R1Player->AttackRange)
		{
			StopMovement();
			R1Player->ActivateAbility(R1GameplayTags::Ability_Attack);
		}
		else
		{
			CacheDestination = TargetAttackActor->GetActorLocation();
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CacheDestination);
		}
	}
	// 🌟 2. 몬스터가 아니라면, 상호작용 가능한 대상(인터페이스 보유)인지 확인!
	else if (IR1InteractionInterface* InteractableTarget = Cast<IR1InteractionInterface>(TargetActor))
	{
		bool bIsInRange = false;

		UPrimitiveComponent* TriggerComp = InteractableTarget->GetInteractTrigger();

		if (TriggerComp)
		{
			// 💡 1순위 검사: 플레이어의 캡슐이 대상의 트리거 컴포넌트와 물리적으로 겹쳐있는가? (Lyra 표준 방식)
			bIsInRange = TriggerComp->IsOverlappingActor(R1Player);

			// 💡 2순위 보정: 길찾기(NavMesh) 오차로 인해 아주 미세하게 트리거 테두리 밖에 멈추는 현상 방지용 (테두리 20.0f 이내 허용)
			if (!bIsInRange)
			{
				FVector ClosestPoint = TriggerComp->Bounds.GetBox().GetClosestPointTo(R1Player->GetActorLocation());
				if (FVector::Dist2D(R1Player->GetActorLocation(), ClosestPoint) < 20.0f)
				{
					bIsInRange = true;
				}
			}
		}
		else
		{
			// 트리거 컴포넌트를 세팅하지 않은 액터를 위한 예비(Fallback) 로직
			FVector ClosestPoint = TargetActor->GetComponentsBoundingBox().GetClosestPointTo(R1Player->GetActorLocation());
			if (FVector::Dist2D(R1Player->GetActorLocation(), ClosestPoint) < 80.0f)
			{
				bIsInRange = true;
			}
		}
		if (bIsInRange)
		{
			AActor* InteractTarget = TargetActor;  // 포인터 저장
			ResetMovementState();
			IR1InteractionInterface::Execute_Interact(InteractTarget, this);  // 저장된 포인터 사용
		}
		else
		{
			// 아직 범위 밖이라면 트리거 중심을 향해 계속 이동
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
	StopMovement();

	bMousePressed = false;
	FollowTime = 0.f;

	TargetActor = nullptr;
	TargetAttackActor = nullptr;

	if (R1Player)
	{
		CacheDestination = R1Player->GetActorLocation();
	}
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

	FVector SpawnLoc = R1Player->GetActorLocation() + FVector(0.f,0.f,50.f);

	// 2. 월드에 아이템 액터 스폰!
	FActorSpawnParameters SpawnParams;
	AR1ItemActor* DroppedItem = GetWorld()->SpawnActor<AR1ItemActor>(ItemActorClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);

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


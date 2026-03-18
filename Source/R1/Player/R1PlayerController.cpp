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
#include "Item/R1InventorySubsystem.h"
#include "Item/R1ItemInstance.h"

#include "Data/R1InputData.h"
#include "R1GameplayTags.h"
#include "UI/R1HUD.h"
#include "Object/R1ItemActor.h"

#include "AbilitySystem/R1AbilitySystemComponent.h"
#include "System/R1EquipmentManagerComponent.h"

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

	// 🌟 1. 이 액터가 우리가 상호작용할 수 있는 대상인가? (인터페이스 상속 여부로 확인!)
		// (질문자님 말씀대로 HitActor->ActorHasTag(FName("Enemy")) || HitActor->ActorHasTag(FName("Item")) 로 하셔도 완벽합니다!)
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

	/*if (LocalHighlightActor == nullptr)
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
	HighlightActor = LocalHighlightActor;*/
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
	
	if (TargetAttackActor)  // ← null 체크 추가
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
	else if (AR1ItemActor* TargetItem = Cast<AR1ItemActor>(TargetActor))
	{
		FVector Direction = TargetItem->GetActorLocation() - R1Player->GetActorLocation();

		if (Direction.Length() < 150.0f)
		{
			StopMovement();
			bMousePressed = false;

			TargetItem->OnLootAttempted(R1Player);
			TargetActor = nullptr;
		}
		else
		{
			CacheDestination = TargetItem->GetActorLocation();
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

void AR1PlayerController::DropItemToWorld(UR1ItemInstance* ItemToDrop)
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
		DroppedItem->InitItem(ItemToDrop->GetItemData(), ItemToDrop->ItemRarity);

		// 4. 인벤토리나 장비창에서 실제 아이템 데이터 삭제
		if (InvenSubsys->GetItemPosition(ItemToDrop) != FIntPoint(-1, -1))
		{
			// 인벤토리에서 버림
			InvenSubsys->Items.Remove(ItemToDrop);
			InvenSubsys->RemoveItemFromGrid(ItemToDrop, InvenSubsys->GetItemPosition(ItemToDrop));
		}
		else
		{
			// (장비창에서 직접 버렸을 경우를 대비한 로직 추가 필요시 여기에 작성)
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


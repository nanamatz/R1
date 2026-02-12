


#include "Player/R1PlayerController.h"
#include "Character/R1Player.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"
#include "System/R1AssetManager.h"
#include "Data/R1InputData.h"
#include "R1GameplayTags.h"
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
}

void AR1PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (const UR1InputData* InputData = UR1AssetManager::GetAssetByName<UR1InputData>("InputData"))
	{
		UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

		auto ActionMoveTo = InputData->FindInputActionByTag(R1GameplayTags::Input_Action_SetDestination);

		EnhancedInputComponent->BindAction(ActionMoveTo, ETriggerEvent::Started, this, &ThisClass::OnInputStarted);
		EnhancedInputComponent->BindAction(ActionMoveTo, ETriggerEvent::Triggered, this, &ThisClass::OnSetDestinationTriggered);
		EnhancedInputComponent->BindAction(ActionMoveTo, ETriggerEvent::Completed, this, &ThisClass::OnSetDestinationReleased);
		EnhancedInputComponent->BindAction(ActionMoveTo, ETriggerEvent::Canceled, this, &ThisClass::OnSetDestinationReleased);

		auto ActionInventoryToggle = InputData->FindInputActionByTag(R1GameplayTags::Input_Action_Inventory);
		EnhancedInputComponent->BindAction(ActionInventoryToggle, ETriggerEvent::Started, this, &ThisClass::OnInventoryToggle);

		//auto ActionInteract = InputData->FindInputActionByTag(R1GameplayTags::Input_Action_Interaction);
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

	if (GetCharacter()->GetMesh()->GetAnimInstance()->Montage_IsPlaying(nullptr) == false && R1Player->GetCreatureState() != ECreatureState::Dead)
	{
		if (R1Player)
		{
			R1Player->SetCreatureState(ECreatureState::Moving);
		}
	}

	ChaseTargetAndAttack();
}

void AR1PlayerController::OnInputStarted()
{
	StopMovement();
	bMousePressed = true;
	TargetActor = HighlightActor;
}

void AR1PlayerController::OnSetDestinationTriggered()
{
	if (R1Player && R1Player->GetCreatureState() == ECreatureState::Skill)
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
		//FVector WorldDirection = (CacheDestination - R1Player->GetActorLocation()).GetSafeNormal();
		//R1Player->AddMovementInput(WorldDirection, 1.0, false);
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CacheDestination);
	}
}

void AR1PlayerController::OnSetDestinationReleased()
{
	bMousePressed = false;

	if (R1Player && R1Player->GetCreatureState() == ECreatureState::Skill)
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
	if (TargetActor == nullptr)
	{
		return;
	}

	if (R1Player && R1Player->GetCreatureState() == ECreatureState::Skill)
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

			R1Player->SetCreatureState(ECreatureState::Skill);
			
			TargetActor = HighlightActor;
		}
		else
		{
			//too far you should move
			CacheDestination = TargetActor->GetActorLocation();
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CacheDestination);

			//기존 방식
			//FVector WorldDirection = Direction.GetSafeNormal();
			//R1Player->AddMovementInput(WorldDirection, 1.0, false);
		}
	}
	
	
}

void AR1PlayerController::SwitchCursorType(FHitResult& OutHit)
{
	AActor* CurrentActorType = OutHit.GetActor();

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
		UpdateInputMode();
	}
}

void AR1PlayerController::OnInventoryToggle()
{
	AR1HUD* MyR1HUD = GetHUD<AR1HUD>();
	if (MyR1HUD)
	{
		MyR1HUD->ToggleInventory();

		//// 입력 모드 및 커서 설정은 여기서 관리하는 것이 좋습니다 (시스템 제어)
		//bool bIsVisible = MyR1HUD->bIsInventoryVisible;
		//UpdateInputMode(bIsVisible);
	}
}

void AR1PlayerController::UpdateInputMode(/*bool bIsUIOpen*/)
{
	FInputModeUIOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = true;
	//if (bIsUIOpen)
	//{
	//	FInputModeGameAndUI InputMode;
	//	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	//	InputMode.SetHideCursorDuringCapture(false);
	//	SetInputMode(InputMode);
	//	bShowMouseCursor = true;
	//}
	//else
	//{
	//	FInputModeGameOnly InputMode;

	//	SetInputMode(InputMode);
	//	// 이 프로젝트에서는 기본적으로 커서를 항상 보여주도록 설정되어 있으므로 true 유지
	//	bShowMouseCursor = true; 
	//}
}

void AR1PlayerController::HandleGameplayEvent(FGameplayTag EventTag)
{
	if (EventTag.MatchesTag(R1GameplayTags::Event_Montage_Attack))
	{
		if(IsValid(TargetAttackActor))
			{
				TargetAttackActor = nullptr;
			}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No Target Actor to attack"));
		}
	}
	//if(EventTag.MatchesTag(R1GameplayTags::Event))
}


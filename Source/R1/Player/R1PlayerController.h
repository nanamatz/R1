

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "R1Define.h"
#include "R1PlayerController.generated.h"

struct FInputActionValue;

/**
 * 
 */
UCLASS()
class R1_API AR1PlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AR1PlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;
public:
	virtual void HandleGameplayEvent(FGameplayTag EventTag);
	void UpdateInputMode(bool bIsUIOpen);
private:
	void OnInputStarted();
	void OnSetDestinationTriggered();
	void OnSetDestinationReleased();

	//ECreatureState GetCreatureState();
	//void SetCreatureState(ECreatureState InState);

	void OnInventoryToggle();

private:
	void TickCursorTrace();
	void ChaseTargetAndAttack();

	void SwitchCursorType(FHitResult& OutHit);

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	float ShortPressThreshold = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<class UNiagaraSystem> FXCursor;

private:
	FVector CacheDestination;
	float FollowTime;
	bool bMousePressed = false;

	bool bInventoryHidden = true;

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<class AR1Character> TargetActor;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<class AR1Character> HighlightActor;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<class AR1Player> R1Player;

public:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<class AR1Character> TargetAttackActor;

	int32 AttackCount = 0;
};

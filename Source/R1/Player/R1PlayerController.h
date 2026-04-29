

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "R1Define.h"
#include "R1PlayerController.generated.h"

struct FInputActionValue;
class AR1Character;
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
	virtual void GetAudioListenerPosition(FVector& OutLocation, FVector& OutFrontDir, FVector& OutRightDir) const override;
public:
	virtual void HandleGameplayEvent(FGameplayTag EventTag);

	void UpdateInputMode(bool bShouldUIOnly);

private:
	void OnInputStarted();
	void OnSetDestinationTriggered();
	void OnSetDestinationReleased();

	void OnInventoryToggle();
	void OnQSkill();
	void OnWSkill();
	void OnESkill();
	void OnRSkill();

public:
	// 🌟 카메라 회전용 입력 플래그

	// 🌟 카메라 회전 입력 함수
	void OnLookClickStarted();
	void OnLookClickReleased();
	void OnLookMouse(const FInputActionValue& Value);
public:
	UFUNCTION(BlueprintCallable)
	void OnGameMenuToggle();

private:
	void TickCursorTrace();
	void ChaseTargetAndAttack();
	void SwitchCursorType(FHitResult& OutHit);

public:
	void PlayerOnDead();
public:
	UFUNCTION()
	void HandlePlayerDead(AR1Character* DeadCharacter, AR1Character* Attacker);

public:
	// 텔레포트 등 강제로 이동을 끊어야 할 때 외부에서 호출할 함수
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void ResetMovementState();

	AR1Character* GetHighlightActor();

public:
	// UI에서 아이템을 월드로 드래그 앤 드롭 했을 때 호출할 함수
	UFUNCTION(BlueprintCallable, Category = "Item")
	void DropItemToWorld(class UR1ItemInstance* ItemToDrop, ER1EquipmentSlot FromEquipSlot = ER1EquipmentSlot::None);

	// 스폰할 아이템 액터 클래스 (블루프린트에서 BP_ItemActor 꽂아주기)
	UPROPERTY(EditDefaultsOnly, Category = "Item")
	TSubclassOf<class AR1ItemActor> ItemActorClass;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	float ShortPressThreshold = 0.3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UNiagaraSystem> FXCursor;

private:
	FVector CacheDestination;
	float FollowTime;
	bool bMousePressed = false;

	bool bInventoryHidden = true;
	bool bIsCameraRotating = false;

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> HighlightActor;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<class AR1Player> R1Player;

public:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<class AR1Character> TargetAttackActor;

	int32 AttackCount = 0;
};

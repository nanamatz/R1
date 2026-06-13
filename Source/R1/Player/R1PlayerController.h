

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
	void OnCharacterStatUIToggle();
	void OnQSkill();
	void OnWSkill();
	void OnESkill();
	void OnRSkill();

	// 🌟 중복 제거: Q/W/E/R 스킬 입력 핸들러의 공통 실행 로직
	void ExecuteSkill(ER1SkillSlot Slot);

	// 🌟 중복 제거: 태그로 입력 액션을 찾고, 없으면 에러 로그를 남긴다(없으면 nullptr 반환).
	const class UInputAction* FindInputActionChecked(const class UR1InputData* InputData, const FGameplayTag& Tag, const TCHAR* DebugName) const;

	// 🌟 중복 제거: 캐스팅(시전) 중이라 입력을 받으면 안 되는 상태인지. (R1Player 널 가드 포함)
	bool IsCasting() const;

public:
	void OnLookClickStarted();
	void OnLookClickReleased();
	void OnLookMouse(const FInputActionValue& Value);
public:
	UFUNCTION(BlueprintCallable)
	void OnGameMenuToggle();
	void OnOptionsUIToggle();


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

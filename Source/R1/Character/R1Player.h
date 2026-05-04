

#pragma once

#include "CoreMinimal.h"
#include "Character/R1Character.h"
#include "R1Player.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMpChangedDelegate, float, Ratio);

UCLASS()
class R1_API AR1Player : public AR1Character
{
	GENERATED_BODY()

public:
	AR1Player( );

	// 🌟 [핵심] 외부(Controller 등)에서 배낭을 열어볼 수 있게 해주는 Getter 함수
	class UR1EquipmentManagerComponent* GetEquipmentComponent() const { return EquipmentManagerComponent; }
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UCameraComponent> Camera;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void InitAbilitySystem() override;

	virtual void OnHealthChanged(float Ratio) override;
protected:
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<class UAIPerceptionStimuliSourceComponent> StimuliSource;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void HandleGameplayEvent(FGameplayTag EventTag) override;

	virtual void OnDead(const TObjectPtr<AR1Character> Attacker) override;

public:
	class UR1AttributeSet* GetR1AttributeSet() const { return CommonAttributeSet; }

public:
	void ActivateAbility(FGameplayTag AbilityTag);

	void AddCameraYaw(float YawDelta);

	UPROPERTY(EditAnywhere, Category = "Camera")
	float RotationSpeed = 3.f;
public:

	UPROPERTY(BlueprintAssignable, Category = "Attributes")
	FOnMpChangedDelegate OnMpChanged;

	void OnManaChanged(float Ratio);
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UWidgetComponent> ExpBarComponent;

	float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TSubclassOf<class UGameplayEffect> LevelUpEffect;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<class AR1Character> CombatTarget;

private:
	void InitExpBar();

protected:
	virtual void InitAttributes() override;

	// 플레이어 전용 초기화 GE (GE_InitPlayerStats 할당)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Init")
	TSubclassOf<class UGameplayEffect> PlayerInitStatEffectClass;

protected:
	// 💡 캐릭터의 몸에 부착될 장비 관리자 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<class UR1EquipmentManagerComponent> EquipmentManagerComponent;

protected:
	// 화면에 필터를 씌워줄 장치
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class UPostProcessComponent> PostProcessComp;

	// 에디터에서 방금 만든 M_LowHealth_PP를 넣을 슬롯
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TObjectPtr<class UMaterialInterface> LowHealthMaterial;

	// 코드로 Intensity 수치를 조절하기 위한 다이내믹 인스턴스
	UPROPERTY()
	TObjectPtr<class UMaterialInstanceDynamic> LowHealthMI;

public:
	// 체력 비율에 따라 화면 붉은기를 조절할 함수
	void UpdateLowHealthEffect(float Ratio);

	void TeleportToRoom(FVector TargetLocation);
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class UR1CameraOcclusionComponent> CameraOcclusionComp;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<class USoundBase> HeartbeatSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<class UAudioComponent> HeartbeatAudioComponent;
};

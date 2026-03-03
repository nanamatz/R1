

#pragma once

#include "CoreMinimal.h"
#include "Character/R1Character.h"
#include "R1Player.generated.h"

/**
 * 
 */

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

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UWidgetComponent> ExpBarComponent;

	float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TSubclassOf<class UGameplayEffect> LevelUpEffect;

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
};



#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/R1HighlightInterface.h"    // 외곽선 효과 인터페이스
#include "Interface/R1InteractionInterface.h"
#include "R1GoldActor.generated.h"

UCLASS()
class R1_API AR1GoldActor : public AActor, public IR1HighlightInterface, public IR1InteractionInterface
{
	GENERATED_BODY()
	
public:
	AR1GoldActor();

protected:
	virtual void BeginPlay() override;

	// 🌟 컴포넌트 구성
	UPROPERTY(VisibleAnywhere, Category = "Gold")
	TObjectPtr<class USphereComponent> SphereComp; // 물리 충돌 및 물리 엔진 루트

	UPROPERTY(VisibleAnywhere, Category = "Gold")
	TObjectPtr<class UStaticMeshComponent> MeshComp; // 동전 뭉치 메시

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gold")
	TObjectPtr<class UWidgetComponent> TooltipWidget;

protected:
	UFUNCTION()
	void OnSphereHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	virtual UPrimitiveComponent* GetInteractTrigger() override;
	void PopEffect();
public:
	// 🌟 1. 하이라이트 인터페이스 구현
	virtual void Highlight() override;
	virtual void UnHighlight() override;

	// 🌟 2. 상호작용 인터페이스 구현 (획득 로직)
	virtual void Interact_Implementation(class AR1PlayerController* Interactor) override;

public:
	// 🌟 3. 이 뭉치의 골드량을 세팅하는 함수 (몬스터가 스폰할 때 호출)
	void SetGoldAmount(int32 Amount);

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void DisablePhysicsAndSetOverlap();

private:
	// 이 뭉치 안에 들어있는 실제 골드 수치
	UPROPERTY(EditAnywhere, Category = "Gold", meta = (AllowPrivateAccess = "true"))
	int32 GoldAmount = 10;
protected:
	void UpdateTooltipUI();

protected:
	bool bHighlighted = false;
};

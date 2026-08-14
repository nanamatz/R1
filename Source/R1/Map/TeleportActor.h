

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/R1HighlightInterface.h"
#include "Interface/R1InteractionInterface.h"
#include "TeleportActor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UArrowComponent;
class USoundBase;
class UNiagaraSystem;

/**
 * 같은 레벨 안에서 플레이어를 다른 TeleportActor 위치로 이동시키는 액터.
 * 클릭 상호작용(IR1InteractionInterface)으로만 동작하며, 도착 시 목적지 액터의 정면 방향을 바라본다.
 */
UCLASS()
class R1_API ATeleportActor : public AActor, public IR1HighlightInterface, public IR1InteractionInterface
{
	GENERATED_BODY()

public:
	ATeleportActor();

	virtual void Interact_Implementation(class AR1PlayerController* Interactor) override;

	// 목적지 계산: 액터 정면(ForwardVector) 방향으로 ExitOffset 만큼 떨어진 지점
	FVector GetExitLocation() const;

	UFUNCTION(BlueprintPure, Category = "Teleport")
	bool IsOnCooldown() const;

protected:
	virtual void BeginPlay() override;
	virtual void Highlight() override;
	virtual void UnHighlight() override;
	virtual UPrimitiveComponent* GetInteractTrigger() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> RootComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> TeleportMesh;

	// 클릭 트레이스 + 플레이어 접근 판정용 트리거
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	// 에디터에서 정면(도착 시 바라볼 방향)을 눈으로 확인하기 위한 화살표
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UArrowComponent> ForwardArrow;

	// 이동할 목적지 텔레포터. 한쪽만 지정하면 BeginPlay에서 반대편 링크를 자동으로 채워 양방향이 된다.
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Teleport")
	TObjectPtr<ATeleportActor> LinkedTeleporter;

	// 목적지 텔레포터 정면으로 얼마나 앞에 내려놓을지 (겹침 방지)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teleport")
	float ExitOffset = 150.0f;

	// 재사용 대기시간(초). 링크된 양쪽이 함께 대기하므로 도착 즉시 되돌아가는 것도 막힌다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teleport")
	float TeleportCooldown = 1.0f;

	// 출발 지점과 도착 지점 양쪽에 재생되는 이펙트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teleport")
	TObjectPtr<UNiagaraSystem> TeleportVFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundBase> TeleportSound;

	// 커스텀 뎁스 스텐실 값. 프로젝트 외곽선 머티리얼이 해석하는 값에 맞춰 조정한다.
	// 252 = 일반 상호작용 색, 250 = 적(캐릭터) 색을 재활용한 "사용 불가" 표시.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Highlight")
	int32 ReadyStencil = 252;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Highlight")
	int32 CooldownStencil = 250;

private:
	// 쿨다운 종료 시점에 커서가 아직 올라가 있으면 외곽선 색을 되돌린다.
	void StartCooldown();
	void RefreshHighlight();

	// 쿨다운이 끝나는 월드 시간. 0이면 사용 가능.
	float CooldownEndTime = 0.0f;

	bool bHighlighted = false;

	FTimerHandle CooldownTimerHandle;
};

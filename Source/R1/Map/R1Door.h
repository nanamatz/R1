

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Map/R1MapGenerator.h"
#include "Interface/R1HighlightInterface.h"
#include "Interface/R1InteractionInterface.h"
#include "R1Door.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDoorEnteredSignature, ER1DoorDirection, DoorDirection);

UCLASS()
class R1_API AR1Door : public AActor, public IR1HighlightInterface, public IR1InteractionInterface
{
	GENERATED_BODY()
public:
	AR1Door();

protected:
	virtual void BeginPlay() override;
	virtual void Highlight() override;
	virtual void UnHighlight() override;
	virtual UPrimitiveComponent* GetInteractTrigger() override;
public:
	// 문의 루트 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> RootComp;

	// 문의 외형 (메시)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> DoorMesh;

	// 플레이어 접근을 감지할 트리거 박스
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	// 이 문이 방의 어느 방향에 위치해 있는지 에디터에서 설정할 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Setup")
	ER1DoorDirection DoorDirection = ER1DoorDirection::None;

	// 이 문이 향하는 다음 방의 NodeID (-1이면 연결된 방이 없는 막힌 문)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door Setup")
	int32 TargetNodeID = -1;

	// 문에 닿았을 때 발생하는 이벤트 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Door Events")
	FOnDoorEnteredSignature OnDoorEntered;

	// 문을 열거나 잠그는 등 상태를 세팅하는 함수
	UFUNCTION(BlueprintCallable, Category = "Door Logic")
	void SetupDoorConnection(int32 InTargetNodeID);

private:
	// 트리거 박스에 플레이어가 겹쳤을 때 실행될 함수
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	// 문의 잠금 상태를 변경하는 함수
	UFUNCTION(BlueprintCallable, Category = "Door Logic")
	void SetLocked(bool bIsLocked);

	// 현재 문이 잠겨있는지 여부 (블루프린트에서 머티리얼을 바꿀 때 쓸 수 있게 ReadOnly로 둡니다)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door Logic")
	bool bLocked = false;

public:
	// 🌟 추가: 이 문을 열기 위해 열쇠가 필요한가?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door Logic")
	bool bRequiresKey = false;

	// 🌟 추가: 문에 자물쇠를 채우는 함수
	UFUNCTION(BlueprintCallable, Category = "Door Logic")
	void SetKeyLocked(bool bNeedsKey);
};

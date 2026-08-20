

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/R1HighlightInterface.h"
#include "Interface/R1InteractionInterface.h"
#include "R1Portal.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class USoundBase;

UCLASS()
class R1_API AR1Portal : public AActor, public IR1HighlightInterface, public IR1InteractionInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AR1Portal();

	virtual void Interact_Implementation(class AR1PlayerController* Interactor) override;
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
	TObjectPtr<UStaticMeshComponent> PortalMesh;

	// 플레이어 접근을 감지할 트리거 박스
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	// 포탈이 생성될 때 위치에서 재생되는 사운드
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundBase> SpawnSound;

	// 포탈에 진입해 다음 층으로 이동할 때 재생되는 사운드 (포탈 파괴 후에도 유지되도록 2D 재생)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundBase> EnterSound;

	// 마지막 층 보스를 잡고 생성된 포탈이 이동시킬 엔딩 레벨.
	// 비워두면 마지막 층에서도 아무 곳으로도 이동하지 않는다.
	// 레벨에 직접 배치한 포탈(DevMap 테스트용)에서 인스턴스별로 덮어쓸 수 있다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ending")
	TSoftObjectPtr<UWorld> EndingLevel;
};

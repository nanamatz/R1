

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "R1Define.h"
#include "Interface/R1HighlightInterface.h"
#include "Interface/R1InteractionInterface.h"
#include "R1ItemActor.generated.h"

UCLASS()
class R1_API AR1ItemActor : public AActor, public IR1HighlightInterface, public IR1InteractionInterface
{
	GENERATED_BODY()
public:
	AR1ItemActor();
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void Highlight() override;
	virtual void UnHighlight() override;

	virtual void Interact_Implementation(class AR1PlayerController* Interactor) override;

	virtual UPrimitiveComponent* GetInteractTrigger() override;

	void PopEffect();

	void DisablePhysicsAndSetOverlap();
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UBoxComponent> BoxComp;

	UFUNCTION()
	void OnBoxHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
public:
	// 🌟 머리 위에 아이템 이름을 띄워줄 UI 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<class UWidgetComponent> TooltipWidget;

public:
	// 던전 매니저가 이 아이템을 스폰한 직후 호출해 줄 초기화 함수
	void InitItem(class UR1ItemAssetData* InItemData, EItemRarity InRarity, int32 InCount = 1); // (추가)

	// 플레이어가 다가와서 루팅(줍기)을 시도할 때 호출할 함수
	void OnLootAttempted(class AR1Player* Looter);
protected:
	void UpdateTooltipUI();
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item")
	int32 ItemCount = 1;

	// 아이템의 3D 외형
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> MeshComp;

	// 내 아이템 정보
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<class UR1ItemAssetData> ItemData; // (추가)

	UPROPERTY(BlueprintReadOnly, Category = "Item")
	EItemRarity ItemRarity = EItemRarity::Common;

protected:
	bool bHighlighted = false;

private:
	void RefreshLocalization();
	// 🌟 2. 통합 전리품 연출(Loot Effect) 나이아가라 컴포넌트 추가
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UNiagaraComponent> HaloEffect;

private:
	static const FName RarityColorParamName; // "User.RarityColor"
	static const FName IsLegendaryParamName; // "User.IsLegendary"
};

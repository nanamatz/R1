

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "R1Define.h"
#include "Interface/R1HighlightInterface.h"
#include "R1ItemActor.generated.h"

UCLASS()
class R1_API AR1ItemActor : public AActor, public IR1HighlightInterface
{
	GENERATED_BODY()
public:
	AR1ItemActor();
protected:
	virtual void BeginPlay() override;

public:
	virtual void Highlight() override;
	virtual void UnHighlight() override;

public:
	// 🌟 머리 위에 아이템 이름을 띄워줄 UI 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<class UWidgetComponent> TooltipWidget;

public:
	// 던전 매니저가 이 아이템을 스폰한 직후 호출해 줄 초기화 함수
	void InitItem(class UR1ItemAssetData* InItemData, EItemRarity InRarity); // (추가)

	// 플레이어가 다가와서 루팅(줍기)을 시도할 때 호출할 함수
	void OnLootAttempted(class AR1Player* Looter);

public:
	// 마우스 클릭 및 상호작용 범위를 감지할 콜리전
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class USphereComponent> SphereComp;

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
};

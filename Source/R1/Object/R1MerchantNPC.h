#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/R1HighlightInterface.h"
#include "R1MerchantNPC.generated.h"

class UR1ItemAssetData;
class UR1ItemPoolData;

UCLASS()
class R1_API AR1MerchantNPC : public AActor, public IR1HighlightInterface
{
	GENERATED_BODY()
	
public:	
	AR1MerchantNPC();

protected:
	virtual void BeginPlay() override;

public:
	// IR1HighlightInterface 구현
	virtual void Highlight() override;
	virtual void UnHighlight() override;

	// 상호작용 함수
	UFUNCTION(BlueprintCallable, Category = "Merchant")
	void OpenShop();

	UFUNCTION(BlueprintCallable, Category = "Merchant")
	const TArray<UR1ItemInstance*>& GetItemsForSale() const { return ItemsForSale; }

protected:
	void GenerateShopItems();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Merchant")
	TObjectPtr<USceneComponent> RootComp;

	UPROPERTY(VisibleAnywhere, Category = "Merchant")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	// 상인이 팔 아이템들 (3개 고정)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Merchant")
	TArray<TObjectPtr<class UR1ItemInstance>> ItemsForSale;

	// 아이템을 뽑아올 풀 (에디터에서 할당)
	UPROPERTY(EditAnywhere, Category = "Merchant")
	TObjectPtr<UR1ItemPoolData> ItemPool;

	UPROPERTY(EditAnywhere, Category = "Merchant")
	TSubclassOf<class UR1ShopWidget> ShopWidgetClass;

	UPROPERTY()
	TObjectPtr<class UR1ShopWidget> ShopWidget;
};

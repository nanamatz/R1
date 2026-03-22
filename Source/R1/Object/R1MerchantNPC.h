#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/R1HighlightInterface.h"
#include "R1MerchantNPC.generated.h"

class UR1ItemAssetData;
class UR1ItemPoolData;
class UR1ItemInstance;

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

	// 🌟 물건이 팔렸을 때 진열대(배열)에서 아이템을 지우는 함수 추가
	UFUNCTION(BlueprintCallable, Category = "Merchant")
	bool RemoveItemFromSale(UR1ItemInstance* ItemToRemove);

protected:
	void GenerateShopItems();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Merchant")
	TObjectPtr<USceneComponent> RootComp;

	UPROPERTY(VisibleAnywhere, Category = "Merchant")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	// 상인이 팔 아이템들 (3개 고정)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Merchant")
	TArray<TObjectPtr<UR1ItemInstance>> ItemsForSale;

	// 아이템을 뽑아올 풀 (에디터에서 할당)
	UPROPERTY(EditAnywhere, Category = "Merchant")
	TObjectPtr<UR1ItemPoolData> ItemPool;
};

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/R1HighlightInterface.h"
#include "Interface/R1InteractionInterface.h"
#include "R1MerchantNPC.generated.h"

class UR1ItemAssetData;
class UR1ItemPoolData;
class UR1ItemInstance;
class UBoxComponent;
class UR1NPCPoolData;

UCLASS()
class R1_API AR1MerchantNPC : public AActor, public IR1HighlightInterface, public IR1InteractionInterface
{
	GENERATED_BODY()
	
public:
	AR1MerchantNPC();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnInteractTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
public:
	// IR1HighlightInterface 구현
	virtual void Highlight() override;
	virtual void UnHighlight() override;
	virtual void Interact_Implementation(class AR1PlayerController* Interactor) override;
	virtual UPrimitiveComponent* GetInteractTrigger() override;

	// 상호작용 함수
	UFUNCTION(BlueprintCallable, Category = "Merchant")
	void OpenShop();

	UFUNCTION(BlueprintCallable, Category = "Merchant")
	const TArray<UR1ItemInstance*>& GetItemsForSale() const { return ItemsForSale; }

	// 🌟 물건이 팔렸을 때 진열대(배열)에서 아이템을 지우는 함수 추가
	UFUNCTION(BlueprintCallable, Category = "Merchant")
	bool RemoveItemFromSale(UR1ItemInstance* ItemToRemove);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop Info")
	TObjectPtr<class UR1NPCPoolData> NPCPool;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shop Info")
	TObjectPtr<class UR1ShopNPCData> CurrentNPCData;

private:
	// 스폰 시 신분을 랜덤으로 배정하는 함수
	void AssignNPCIdentity();
protected:
	void GenerateShopItems();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Merchant")
	TObjectPtr<USceneComponent> RootComp;

	UPROPERTY(VisibleAnywhere, Category = "Merchant")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Merchant")
	TObjectPtr<UBoxComponent> InteractTrigger;

	// 상인이 팔 아이템들 (3개 고정)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Merchant")
	TArray<TObjectPtr<UR1ItemInstance>> ItemsForSale;

	// 아이템을 뽑아올 풀 (에디터에서 할당)
	UPROPERTY(EditAnywhere, Category = "Merchant")
	TObjectPtr<UR1ItemPoolData> ItemPool;
private:
	UPROPERTY(EditAnywhere)
	int32 CuratedItemCounts = 5;

protected:
	// 현재 NPC가 속한 방의 ID를 찾아냅니다.
	int32 GetRoomNodeID();
};

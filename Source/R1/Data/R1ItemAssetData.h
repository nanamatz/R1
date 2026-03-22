

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "R1Define.h"
#include "R1GameplayTags.h"
#include "R1ItemAssetData.generated.h"

class UR1GameplayAbility;
class UGameplayEffect;
/**
 * 
 */
UCLASS()
class R1_API UR1ItemAssetData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Asset Manager에서 관리하기 쉽도록 PrimaryAssetId를 생성해 줍니다.
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(FName("ItemData"), GetFName());
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	EItemRarity ItemRarity = EItemRarity::Common;

	// 🌟 [핵심] 희귀도에 따라 가중치를 스스로 계산해서 반환하는 함수
	// (기획에 맞게 수치는 자유롭게 조절하세요. 높을수록 잘 나옵니다.)
	float GetDropWeight() const
	{
		switch (ItemRarity)
		{
		case EItemRarity::Common:		return 1000.0f; // 70% 확률 수준
		case EItemRarity::Uncommon:		return 300.0f;  // 20%
		case EItemRarity::Rare:			return 100.0f;  // 7%
		case EItemRarity::Epic:			return 30.0f;   // 2%
		case EItemRarity::Legendary:	return 5.0f;    // 0.X%대 극악의 확률
		default:						return 0.0f;
		}
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FName ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	TObjectPtr<class UTexture2D> ItemIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	TObjectPtr<class UStaticMesh> ItemMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FIntPoint ItemSize = FIntPoint(1, 1);

	// (이전 답변에서 정의한 열거형)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	ER1ItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	int32 BaseValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	TArray<ER1EquipmentSlot> EquipSlots;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	TSubclassOf<UGameplayEffect> EquipStatEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	TArray<TSubclassOf<UGameplayEffect>> GrantedEffects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	TArray<TSubclassOf<UR1GameplayAbility>> GrantedAbilities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	TMap<FGameplayTag, float> StatModifiers;
};

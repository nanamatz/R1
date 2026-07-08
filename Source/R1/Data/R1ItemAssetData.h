

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

	float GetDropWeight() const
	{
		switch (ItemRarity)
		{
		case EItemRarity::Common:		return 900.0f; 
		case EItemRarity::Uncommon:		return 550.0f;  
		case EItemRarity::Rare:			return 300.0f;  
		case EItemRarity::Epic:			return 100.0f;   
		case EItemRarity::Legendary:	return 50.0f;   
		default:						return 0.0f;
		}
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FName ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FName LocalizationKey;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic", meta = (MultiLine = true))
	TArray<FText> EffectDescriptions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	TArray<ER1EquipmentSlot> EquipSlots;

	// 무기 슬롯 아이템에서만 의미 있음. 비무기 아이템은 기본값(Unarmed) 유지.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	ER1WeaponType WeaponType = ER1WeaponType::Unarmed;

	// 아이템의 속성 (향후 저항/속성 데미지 판정용 게임플레이 정체성)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	ER1ElementType ElementType = ER1ElementType::None;

	// 장착 중 무기 메시에 붙는 루프 이펙트 (없으면 미표시)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Visual")
	TSoftObjectPtr<class UNiagaraSystem> WeaponAuraVFX;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	TSubclassOf<UGameplayEffect> EquipStatEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	TArray<TSubclassOf<UGameplayEffect>> GrantedEffects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	TArray<TSubclassOf<UR1GameplayAbility>> GrantedAbilities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	TMap<FGameplayTag, float> StatModifiers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Visual")
	FName EquipSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|UI")
	TObjectPtr<USoundBase> InventoryDropSound;  

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|UI")
	TObjectPtr<USoundBase> EquipSound;  

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|UI")
	TObjectPtr<USoundBase> UnEquipSound;  

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Gameplay")
	TObjectPtr<USoundBase> WorldLootingSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Gameplay")
	TObjectPtr<USoundBase> WorldDropSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Audio")
	TMap<FGameplayTag, TSoftObjectPtr<USoundBase>> AudioRoutingMap;
};

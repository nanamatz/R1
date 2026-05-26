


#include "Library/R1ItemFunctionLibrary.h"
#include "System/R1LocalizationSubsystem.h"
#include "R1GameplayTags.h"

FSlateColor UR1ItemFunctionLibrary::GetRarityColor(EItemRarity Rarity)
{
	// 💡 "쨍한 원색"의 채도(S)와 명도(V)를 낮춰 눈이 편안한 고급스러운 톤으로 조정했습니다.
	switch (Rarity)
	{
	case EItemRarity::Common:
		return FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)); // 차분한 회색
	case EItemRarity::Uncommon:
		return FSlateColor(FLinearColor(0.3f, 0.6f, 0.3f)); // 부드러운 초록
	case EItemRarity::Rare:
		return FSlateColor(FLinearColor(0.2f, 0.4f, 0.8f)); // 차분한 파랑
	case EItemRarity::Epic:
		return FSlateColor(FLinearColor(0.6f, 0.3f, 0.7f)); // 우아한 보라
	case EItemRarity::Legendary:
		return FSlateColor(FLinearColor(1.0f, 0.5f, 0.1f)); // 묵직한 금색
	}
	return FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f));
}

static UR1LocalizationSubsystem* GetLocSub(UObject* WorldContextObject)
{
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<UR1LocalizationSubsystem>() : nullptr;
}

FText UR1ItemFunctionLibrary::GetRarityText(UObject* WorldContextObject, EItemRarity Rarity)
{
	FName Key = NAME_None;
	FText Fallback;
	switch (Rarity)
	{
	case EItemRarity::Common:    Key = "Rarity_Common";    Fallback = FText::FromString(TEXT("Common"));    break;
	case EItemRarity::Uncommon:  Key = "Rarity_Uncommon";  Fallback = FText::FromString(TEXT("Uncommon"));  break;
	case EItemRarity::Rare:      Key = "Rarity_Rare";      Fallback = FText::FromString(TEXT("Rare"));      break;
	case EItemRarity::Epic:      Key = "Rarity_Epic";      Fallback = FText::FromString(TEXT("Epic"));      break;
	case EItemRarity::Legendary: Key = "Rarity_Legendary"; Fallback = FText::FromString(TEXT("Legendary")); break;
	default: return FText::GetEmpty();
	}
	UR1LocalizationSubsystem* LocSub = GetLocSub(WorldContextObject);
	return LocSub ? LocSub->GetText(Key) : Fallback;
}

FText UR1ItemFunctionLibrary::GetItemTypeText(UObject* WorldContextObject, ER1ItemType ItemType)
{
	FName Key = NAME_None;
	FText Fallback;
	switch (ItemType)
	{
	case ER1ItemType::Equipment:  Key = "ItemType_Equipment";  Fallback = FText::FromString(TEXT("Equipment"));  break;
	case ER1ItemType::Consumable: Key = "ItemType_Consumable"; Fallback = FText::FromString(TEXT("Consumable")); break;
	case ER1ItemType::Material:   Key = "ItemType_Material";   Fallback = FText::FromString(TEXT("Material"));   break;
	case ER1ItemType::Key:        Key = "ItemType_Key";        Fallback = FText::FromString(TEXT("Key"));        break;
	default: return FText::GetEmpty();
	}
	UR1LocalizationSubsystem* LocSub = GetLocSub(WorldContextObject);
	return LocSub ? LocSub->GetText(Key) : Fallback;
}

FText UR1ItemFunctionLibrary::GetEquipSlotText(UObject* WorldContextObject, ER1EquipmentSlot EquipSlot)
{
	FName Key = NAME_None;
	FText Fallback;
	switch (EquipSlot)
	{
	case ER1EquipmentSlot::Weapon: Key = "EquipSlot_Weapon"; Fallback = FText::FromString(TEXT("Weapon")); break;
	case ER1EquipmentSlot::Helmet: Key = "EquipSlot_Helmet"; Fallback = FText::FromString(TEXT("Helm"));   break;
	case ER1EquipmentSlot::Armor:  Key = "EquipSlot_Armor";  Fallback = FText::FromString(TEXT("Armor"));  break;
	case ER1EquipmentSlot::Glove:  Key = "EquipSlot_Glove";  Fallback = FText::FromString(TEXT("Glove"));  break;
	case ER1EquipmentSlot::Ring1:
	case ER1EquipmentSlot::Ring2:  Key = "EquipSlot_Ring";   Fallback = FText::FromString(TEXT("Ring"));   break;
	case ER1EquipmentSlot::Boots:  Key = "EquipSlot_Boots";  Fallback = FText::FromString(TEXT("Boots"));  break;
	default: return FText::GetEmpty();
	}
	UR1LocalizationSubsystem* LocSub = GetLocSub(WorldContextObject);
	return LocSub ? LocSub->GetText(Key) : Fallback;
}

FName UR1ItemFunctionLibrary::GetStatLocalizationKey(const FGameplayTag& Tag)
{
	using namespace R1GameplayTags;
	if (Tag == Data_Attribute_MaxHealth)             return "Stat_MaxHealth";
	if (Tag == Data_Attribute_Health)                return "Stat_Health";
	if (Tag == Data_Attribute_HealthRegeneration)    return "Stat_HealthRegen";
	if (Tag == Data_Attribute_MoveSpeed)             return "Stat_MoveSpeed";
	if (Tag == Data_Attribute_BaseDamage)            return "Stat_BaseDamage";
	if (Tag == Data_Attribute_BaseDefence)           return "Stat_BaseDefence";
	if (Tag == Data_Attribute_AttackRange)           return "Stat_AttackRange";
	if (Tag == Data_Attribute_AttackRadius)          return "Stat_AttackRadius";
	if (Tag == Data_Attribute_AttackSpeed)           return "Stat_AttackSpeed";
	if (Tag == Data_Attribute_CriticalHitChance)     return "Stat_CritChance";
	if (Tag == Data_Attribute_CriticalHitMultiplier) return "Stat_CritMultiplier";
	if (Tag == Data_Attribute_MaxMana)               return "Stat_MaxMana";
	if (Tag == Data_Attribute_Mana)                  return "Stat_Mana";
	if (Tag == Data_Attribute_ManaRegeneration)      return "Stat_ManaRegen";
	if (Tag == Data_Attribute_WeaponDamage)          return "Stat_WeaponDamage";
	if (Tag == Data_Attribute_EquipDefence)          return "Stat_EquipDefence";
	if (Tag == Data_Attribute_DamageMultiplier)      return "Stat_DamageMultiplier";
	if (Tag == Data_Attribute_DefenceMultiplier)     return "Stat_DefenceMultiplier";
	if (Tag == Data_Attribute_Ability)               return "Stat_Ability";
	return NAME_None;
}
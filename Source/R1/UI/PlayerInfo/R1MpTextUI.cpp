#include "UI/PlayerInfo/R1MpTextUI.h"
#include "R1LogChannels.h"

#include "Components/TextBlock.h"
#include "Player/R1PlayerState.h"
#include "AbilitySystem/Attribute/PlayerAttributeSet.h"
#include "AbilitySystemComponent.h"

void UR1MpTextUI::NativeConstruct()
{
	Super::NativeConstruct();

	AR1PlayerState* PS = Cast<AR1PlayerState>(GetOwningPlayerState());
	if (!PS)
	{
		UE_LOG(LogR1, Error, TEXT("R1MpTextUI: Failed to find AR1PlayerState during NativeConstruct!"));
		return;
	}

	BoundASC = PS->GetAbilitySystemComponent();
	if (!BoundASC)
	{
		UE_LOG(LogR1, Error, TEXT("R1MpTextUI: AR1PlayerState has no AbilitySystemComponent!"));
		return;
	}

	BoundASC->GetGameplayAttributeValueChangeDelegate(UPlayerAttributeSet::GetManaAttribute())
		.AddUObject(this, &UR1MpTextUI::OnMpAttributeChanged);
	BoundASC->GetGameplayAttributeValueChangeDelegate(UPlayerAttributeSet::GetMaxManaAttribute())
		.AddUObject(this, &UR1MpTextUI::OnMpAttributeChanged);

	RefreshText();
}

void UR1MpTextUI::NativeDestruct()
{
	if (BoundASC)
	{
		BoundASC->GetGameplayAttributeValueChangeDelegate(UPlayerAttributeSet::GetManaAttribute()).RemoveAll(this);
		BoundASC->GetGameplayAttributeValueChangeDelegate(UPlayerAttributeSet::GetMaxManaAttribute()).RemoveAll(this);
		BoundASC = nullptr;
	}

	Super::NativeDestruct();
}

void UR1MpTextUI::OnMpAttributeChanged(const FOnAttributeChangeData& /*Data*/)
{
	RefreshText();
}

void UR1MpTextUI::RefreshText()
{
	if (!Text_Mp || !BoundASC)
	{
		return;
	}

	const int32 Mana = FMath::FloorToInt(BoundASC->GetNumericAttribute(UPlayerAttributeSet::GetManaAttribute()));
	const int32 MaxMana = FMath::FloorToInt(BoundASC->GetNumericAttribute(UPlayerAttributeSet::GetMaxManaAttribute()));

	const FText Formatted = FText::Format(
		NSLOCTEXT("R1MpTextUI", "MpFraction", "{0} / {1}"),
		FText::AsNumber(Mana),
		FText::AsNumber(MaxMana));

	Text_Mp->SetText(Formatted);
}

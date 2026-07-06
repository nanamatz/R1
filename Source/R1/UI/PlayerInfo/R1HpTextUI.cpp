#include "UI/PlayerInfo/R1HpTextUI.h"
#include "R1LogChannels.h"

#include "Components/TextBlock.h"
#include "Character/R1Player.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "AbilitySystemComponent.h"

void UR1HpTextUI::NativeConstruct()
{
	Super::NativeConstruct();

	AR1Player* Player = Cast<AR1Player>(GetOwningPlayerPawn());
	if (!Player)
	{
		UE_LOG(LogR1, Error, TEXT("R1HpTextUI: Failed to find AR1Player during NativeConstruct!"));
		return;
	}

	BoundASC = Player->GetAbilitySystemComponent();
	if (!BoundASC)
	{
		UE_LOG(LogR1, Error, TEXT("R1HpTextUI: AR1Player has no AbilitySystemComponent!"));
		return;
	}

	BoundASC->GetGameplayAttributeValueChangeDelegate(UR1AttributeSet::GetHealthAttribute())
		.AddUObject(this, &UR1HpTextUI::OnHpAttributeChanged);
	BoundASC->GetGameplayAttributeValueChangeDelegate(UR1AttributeSet::GetMaxHealthAttribute())
		.AddUObject(this, &UR1HpTextUI::OnHpAttributeChanged);

	RefreshText();
}

void UR1HpTextUI::NativeDestruct()
{
	if (BoundASC)
	{
		BoundASC->GetGameplayAttributeValueChangeDelegate(UR1AttributeSet::GetHealthAttribute()).RemoveAll(this);
		BoundASC->GetGameplayAttributeValueChangeDelegate(UR1AttributeSet::GetMaxHealthAttribute()).RemoveAll(this);
		BoundASC = nullptr;
	}

	Super::NativeDestruct();
}

void UR1HpTextUI::OnHpAttributeChanged(const FOnAttributeChangeData& /*Data*/)
{
	RefreshText();
}

void UR1HpTextUI::RefreshText()
{
	if (!Text_Hp || !BoundASC)
	{
		return;
	}

	const int32 Health = FMath::FloorToInt(BoundASC->GetNumericAttribute(UR1AttributeSet::GetHealthAttribute()));
	const int32 MaxHealth = FMath::FloorToInt(BoundASC->GetNumericAttribute(UR1AttributeSet::GetMaxHealthAttribute()));

	const FText Formatted = FText::Format(
		NSLOCTEXT("R1HpTextUI", "HpFraction", "{0} / {1}"),
		FText::AsNumber(Health),
		FText::AsNumber(MaxHealth));

	Text_Hp->SetText(Formatted);
}

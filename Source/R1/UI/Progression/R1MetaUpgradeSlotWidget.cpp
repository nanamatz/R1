


#include "UI/Progression/R1MetaUpgradeSlotWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UR1MetaUpgradeSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (Button_Upgrade)
	{
		Button_Upgrade->OnClicked.AddDynamic(this, &UR1MetaUpgradeSlotWidget::OnButtonClicked);
	}
}

void UR1MetaUpgradeSlotWidget::InitSlot(FGameplayTag InTag, const FText& InName, int32 InCurrentLevel, int32 InMaxLevel, bool bCanAfford)
{
	MyUpgradeTag = InTag;

	if (Text_SkillName) Text_SkillName->SetText(InName);

	if (Text_Level)
	{
		FString LevelStr = FString::Printf(TEXT("Lv. %d / %d"), InCurrentLevel, InMaxLevel);
		Text_Level->SetText(FText::FromString(LevelStr));
	}

	if (Button_Upgrade)
	{
		// 만렙이거나 돈(포인트)이 없으면 버튼 비활성화!
		bool bIsMaxLevel = (InCurrentLevel >= InMaxLevel);
		Button_Upgrade->SetIsEnabled(!bIsMaxLevel && bCanAfford);
	}
}

void UR1MetaUpgradeSlotWidget::OnButtonClicked()
{
	if (OnUpgradeButtonClicked.IsBound())
	{
		OnUpgradeButtonClicked.Broadcast(MyUpgradeTag);
	}
}

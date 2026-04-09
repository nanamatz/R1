


#include "UI/Progression/R1MetaUpgradeSlotWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Engine/Texture2D.h"

void UR1MetaUpgradeSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (Button_Upgrade)
	{
		Button_Upgrade->OnClicked.AddDynamic(this, &UR1MetaUpgradeSlotWidget::OnButtonClicked);
	}
}

void UR1MetaUpgradeSlotWidget::InitSlot(FGameplayTag InTag, const FText& InName, int32 InCurrentLevel, int32 InMaxLevel, UTexture2D* IconImage, bool bCanAfford)
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

		if (IconImage)
		{
			// 버튼의 현재 스타일을 복사해 옵니다.
			FButtonStyle NewStyle = Button_Upgrade->GetStyle();

			// Normal, Hovered, Pressed 상태의 텍스처를 모두 새 아이콘으로 덮어씌웁니다.
			NewStyle.Normal.SetResourceObject(IconImage);
			NewStyle.Hovered.SetResourceObject(IconImage);
			NewStyle.Pressed.SetResourceObject(IconImage);

			// 변경된 스타일을 버튼에 다시 적용합니다.
			Button_Upgrade->SetStyle(NewStyle);
		}
	}
}

void UR1MetaUpgradeSlotWidget::OnButtonClicked()
{
	if (OnUpgradeButtonClicked.IsBound())
	{
		OnUpgradeButtonClicked.Broadcast(MyUpgradeTag);
	}
}

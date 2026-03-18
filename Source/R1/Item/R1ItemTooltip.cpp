


#include "Item/R1ItemTooltip.h"
#include "Components/TextBlock.h"
#include "R1Define.h"

void UR1ItemTooltip::SetItemInfo(const FText& InName, EItemRarity InRarity)
{
	if (!Text_ItemName) return;

	// 1. 이름 텍스트 적용
	Text_ItemName->SetText(InName);

	// 2. 희귀도별 색상 판별 (기본값은 일반 등급의 흰색/밝은 회색)
	FSlateColor TextColor = FSlateColor(FLinearColor(0.9f, 0.9f, 0.9f));

	switch (InRarity)
	{
	case EItemRarity::Common: // 일반 (흰색/회색)
		TextColor = FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f));
		break;

	case EItemRarity::Uncommon: // 고급 (초록색)
		TextColor = FSlateColor(FLinearColor(0.1f, 0.8f, 0.1f));
		break;

	case EItemRarity::Rare: // 희귀 (파란색)
		TextColor = FSlateColor(FLinearColor(0.0f, 0.5f, 1.0f));
		break;

	case EItemRarity::Epic: // 영웅 (보라색)
		TextColor = FSlateColor(FLinearColor(0.7f, 0.0f, 1.0f));
		break;

	case EItemRarity::Legendary: // 전설 (주황/금색)
		TextColor = FSlateColor(FLinearColor(1.0f, 0.5f, 0.0f));
		break;
	default:
		TextColor = FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f));
	}

	// 3. 텍스트 블록에 색상 적용!
	Text_ItemName->SetColorAndOpacity(TextColor);
}

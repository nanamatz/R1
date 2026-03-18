


#include "Item/R1ItemTooltip.h"
#include "Components/TextBlock.h"

void UR1ItemTooltip::SetItemNameText(const FText& InName)
{
	if (Text_ItemName)
	{
		Text_ItemName->SetText(InName); // 전달받은 이름으로 텍스트 변경!
	}
}




#include "UI/System/R1FloorGuideWidget.h"
#include "Components/TextBlock.h"

void UR1FloorGuideWidget::PlayAnnouncement(ER1FloorLevel FloorLevel)
{
	if (Text_FloorName)
	{
		// Enum을 텍스트로 변환해서 꽂아줍니다.
		Text_FloorName->SetText(GetFloorText(FloorLevel));
	}

	if (AppearingAnimation)
	{
		PlayAnimation(AppearingAnimation);
	}

	if (FadeInOutAnimation)
	{
		PlayAnimation(FadeInOutAnimation);
	}

}

FText UR1FloorGuideWidget::GetFloorText(ER1FloorLevel FloorLevel)
{
	switch (FloorLevel)
	{
	case ER1FloorLevel::Laboratory:     return FText::FromString(TEXT("Laboratory"));
	case ER1FloorLevel::Factory:        return FText::FromString(TEXT("Factory"));
	case ER1FloorLevel::SecurityArea:   return FText::FromString(TEXT("Security Area"));
	case ER1FloorLevel::RestrictedArea: return FText::FromString(TEXT("Restricted Area"));
	case ER1FloorLevel::Robby:          return FText::FromString(TEXT("Robby"));
	case ER1FloorLevel::Basement:       return FText::FromString(TEXT("Basement"));
	case ER1FloorLevel::Ground:         return FText::FromString(TEXT("Ground"));
	case ER1FloorLevel::EmptyHall:      return FText::FromString(TEXT("Empty Hall"));
	case ER1FloorLevel::Rooftop:        return FText::FromString(TEXT("Rooftop"));
	default:                            return FText::FromString(TEXT("Unknown Area"));
	}
}

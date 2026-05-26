#include "UI/System/R1FloorGuideWidget.h"
#include "Components/TextBlock.h"
#include "System/R1LocalizationSubsystem.h"

void UR1FloorGuideWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UR1LocalizationSubsystem* LocSub = GI->GetSubsystem<UR1LocalizationSubsystem>())
		{
			LocSub->OnLanguageChanged.AddUObject(this, &UR1FloorGuideWidget::RefreshLocalization);
		}
	}
}

void UR1FloorGuideWidget::NativeDestruct()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UR1LocalizationSubsystem* LocSub = GI->GetSubsystem<UR1LocalizationSubsystem>())
		{
			LocSub->OnLanguageChanged.RemoveAll(this);
		}
	}
	Super::NativeDestruct();
}

void UR1FloorGuideWidget::PlayAnnouncement(ER1FloorLevel FloorLevel)
{
	LastAnnouncedFloor = FloorLevel;

	if (Text_FloorName)
	{
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

void UR1FloorGuideWidget::RefreshLocalization()
{
	if (Text_FloorName)
	{
		Text_FloorName->SetText(GetFloorText(LastAnnouncedFloor));
	}
}

FText UR1FloorGuideWidget::GetFloorText(ER1FloorLevel FloorLevel)
{
	UGameInstance* GI = GetGameInstance();
	UR1LocalizationSubsystem* LocSub = GI ? GI->GetSubsystem<UR1LocalizationSubsystem>() : nullptr;

	FName Key;
	switch (FloorLevel)
	{
	case ER1FloorLevel::Laboratory:     Key = TEXT("Floor_Laboratory");     break;
	case ER1FloorLevel::Factory:        Key = TEXT("Floor_Factory");        break;
	case ER1FloorLevel::SecurityArea:   Key = TEXT("Floor_SecurityArea");   break;
	case ER1FloorLevel::RestrictedArea: Key = TEXT("Floor_RestrictedArea"); break;
	case ER1FloorLevel::Robby:          Key = TEXT("Floor_Robby");          break;
	case ER1FloorLevel::Basement:       Key = TEXT("Floor_Basement");       break;
	case ER1FloorLevel::Ground:         Key = TEXT("Floor_Ground");         break;
	case ER1FloorLevel::EmptyHall:      Key = TEXT("Floor_EmptyHall");      break;
	case ER1FloorLevel::Rooftop:        Key = TEXT("Floor_Rooftop");        break;
	default:                            Key = TEXT("Floor_Unknown");        break;
	}

	return LocSub ? LocSub->GetText(Key) : FText::FromName(Key);
}

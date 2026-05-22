


#include "UI/R1CommonButton.h"
#include "Components/TextBlock.h"
#include "System/R1LocalizationSubsystem.h"

void UR1CommonButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UR1LocalizationSubsystem* LocSub = GI->GetSubsystem<UR1LocalizationSubsystem>())
		{
			LocSub->OnLanguageChanged.AddUObject(this, &UR1CommonButton::RefreshLocalization);
		}
	}

	RefreshLocalization();
}

void UR1CommonButton::NativeDestruct()
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

void UR1CommonButton::RefreshLocalization()
{
	if (!ButtonName || LocalizationKey.IsNone()) return;

	UGameInstance* GI = GetGameInstance();
	UR1LocalizationSubsystem* LocSub = GI ? GI->GetSubsystem<UR1LocalizationSubsystem>() : nullptr;
	if (LocSub)
	{
		ButtonName->SetText(LocSub->GetText(LocalizationKey));
	}
}

void UR1CommonButton::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (ButtonName)
	{
		ButtonName->SetText(ButtonText);
	}
}


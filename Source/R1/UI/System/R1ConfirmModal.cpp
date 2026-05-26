
#include "UI/System/R1ConfirmModal.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "UI/R1CommonButton.h"
#include "System/R1LocalizationSubsystem.h"

void UR1ConfirmModal::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_OK && Button_OK->CommonButton)
	{
		Button_OK->CommonButton->OnClicked.AddDynamic(this, &UR1ConfirmModal::OnConfirmClicked);
	}

	if (Button_Cancel && Button_Cancel->CommonButton)
	{
		Button_Cancel->CommonButton->OnClicked.AddDynamic(this, &UR1ConfirmModal::OnCancelClicked);
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UR1LocalizationSubsystem* LocSub = GI->GetSubsystem<UR1LocalizationSubsystem>())
		{
			LocSub->OnLanguageChanged.AddUObject(this, &UR1ConfirmModal::RefreshLocalization);
		}
	}

	RefreshLocalization();
}

void UR1ConfirmModal::NativeDestruct()
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

void UR1ConfirmModal::RefreshLocalization()
{
	UGameInstance* GI = GetGameInstance();
	UR1LocalizationSubsystem* LocSub = GI ? GI->GetSubsystem<UR1LocalizationSubsystem>() : nullptr;
	if (!LocSub) return;

	if (Text_Message_Title)    Text_Message_Title->SetText(LocSub->GetText("Modal_Title"));
	if (Text_Message_SubTitle) Text_Message_SubTitle->SetText(LocSub->GetText("Modal_SubTitle"));
}

void UR1ConfirmModal::SetMessage(const FText& Message)
{
	if (Text_Message)
	{
		Text_Message->SetText(Message);
	}
}

void UR1ConfirmModal::OnConfirmClicked()
{
	OnConfirm.Broadcast();
	RemoveFromParent();
}

void UR1ConfirmModal::OnCancelClicked()
{
	OnCancel.Broadcast();
	RemoveFromParent();
}


#include "UI/System/R1ConfirmModal.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "UI/R1CommonButton.h"

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

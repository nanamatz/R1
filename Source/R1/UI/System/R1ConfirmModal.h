
#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1ConfirmModal.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnModalConfirmSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnModalCancelSignature);

/**
 * 확인/취소 모달 팝업 위젯
 */
UCLASS()
class R1_API UR1ConfirmModal : public UR1UserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "R1|Events")
	FOnModalConfirmSignature OnConfirm;

	UPROPERTY(BlueprintAssignable, Category = "R1|Events")
	FOnModalCancelSignature OnCancel;

	UFUNCTION(BlueprintCallable, Category = "R1|UI")
	void SetMessage(const FText& Message);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void RefreshLocalization();

protected:
	UFUNCTION()
	void OnConfirmClicked();

	UFUNCTION()
	void OnCancelClicked();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_Message;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_Message_Title;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_Message_SubTitle;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1CommonButton> Button_OK;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UR1CommonButton> Button_Cancel;
};

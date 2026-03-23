

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "R1InventoryWidget.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1InventoryWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;

	// 🌟 블루프린트의 텍스트 컴포넌트 이름과 동일해야 합니다!
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_Gold;

	// 🌟 유저님이 만드신 FOnGoldChanged의 매개변수(int32)와 똑같이 맞춘 수신 함수
	UFUNCTION()
	void UpdateGoldUI(int32 NewGold);
};

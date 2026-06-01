#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "R1MpTextUI.generated.h"

struct FOnAttributeChangeData;

/**
 * MP/MaxMP 텍스트 오버레이. MP Orb에 마우스를 올리면 표시된다.
 * 플레이어스테이트 ASC의 Mana/MaxMana 어트리뷰트 변경 델리게이트에 바인딩해 실시간 갱신.
 */
UCLASS()
class R1_API UR1MpTextUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	// 어트리뷰트 변경 시 호출 (Mana 또는 MaxMana)
	void OnMpAttributeChanged(const FOnAttributeChangeData& Data);

	// 현재 ASC 값을 읽어 "Mana / MaxMana" 텍스트로 갱신
	void RefreshText();

private:
	// WBP에 "Text_Mp" 이름으로 배치된 TextBlock (이름 일치 필수)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_Mp;

	// 바인딩한 ASC (NativeDestruct에서 해제용)
	UPROPERTY()
	TObjectPtr<class UAbilitySystemComponent> BoundASC;
};

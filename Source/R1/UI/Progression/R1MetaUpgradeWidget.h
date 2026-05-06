

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "GameplayTagContainer.h"
#include "R1MetaUpgradeWidget.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1MetaUpgradeWidget : public UR1UserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	// 화면을 켤 때마다 최신 세이브 데이터를 바탕으로 갱신
	UFUNCTION(BlueprintCallable, Category = "MetaUI")
	void RefreshUI();

protected:
	// 슬롯에서 클릭 이벤트가 발생했을 때 처리할 함수
	UFUNCTION()
	void HandleUpgradeRequest(FGameplayTag RequestedTag);

protected:
	// UI 위에 띄울 남은 스킬 포인트 텍스트
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_AvailablePoints;

	// UI 위에 띄울 플레이어 진행 레벨
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_MetaPlayerLevel;

	// 슬롯들을 동적으로 담을 박스 (ScrollBox나 WrapBox 사용 권장)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UUniformGridPanel> Panel_SkillList;

	// 에디터에서 할당할 슬롯 위젯의 클래스 (WBP_MetaUpgradeSlot)
	UPROPERTY(EditDefaultsOnly, Category = "MetaUI")
	TSubclassOf<class UR1MetaUpgradeSlotWidget> SlotWidgetClass;

	// 우리가 만들었던 메타 데이터 테이블 (메뉴판)
	UPROPERTY(EditDefaultsOnly, Category = "MetaUI")
	TObjectPtr<class UDataTable> MetaDataTable;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> Button_Back;

protected:
	UFUNCTION()
	void OnButtonBackClicked();

private:
	int32 GridColumn = 4;
	int32 GridRow = 3;
};

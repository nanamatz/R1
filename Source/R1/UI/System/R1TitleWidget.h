

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1TitleWidget.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1TitleWidget : public UR1UserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;

	// 🌟 키보드 입력을 감지하는 함수
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	// 🌟 마우스 클릭을 감지하는 함수
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

public:
	// 블루프린트에서 할당해줄 '메인 메뉴' 위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> MainMenuWidgetClass;

private:
	bool bIsTransitioning = false; // 중복 실행 방지 플래그

	// 메인 메뉴로 넘어가는 실제 처리 로직
	void TransitionToMainMenu();
};

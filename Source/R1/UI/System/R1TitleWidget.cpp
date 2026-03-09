


#include "UI/System/R1TitleWidget.h"
#include "Blueprint/UserWidget.h"

void UR1TitleWidget::NativeConstruct()
{
	Super::NativeConstruct();

	bIsFocusable = true;
	SetKeyboardFocus();
}

FReply UR1TitleWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// 키보드 아무 키나 누르면 실행
	TransitionToMainMenu();

	// 입력 처리가 완료되었음을 엔진에 알림
	return FReply::Handled();
}

FReply UR1TitleWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 마우스 아무 버튼이나 누르면 실행
	TransitionToMainMenu();
	return FReply::Handled();
}

void UR1TitleWidget::TransitionToMainMenu()
{
	// 이미 넘어가고 있다면 무시 (따닥! 더블 클릭 방지)
	if (bIsTransitioning) return;
	bIsTransitioning = true;

	// 1. 메인 메뉴 위젯을 생성해서 화면에 띄웁니다.
	if (MainMenuWidgetClass)
	{
		UUserWidget* MainMenu = CreateWidget<UUserWidget>(GetWorld(), MainMenuWidgetClass);
		if (MainMenu)
		{
			MainMenu->AddToViewport();
		}
	}

	RemoveFromParent();
}

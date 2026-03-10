
#include "UI/System/R1GameOverWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "System/R1SaveSystem.h"
#include "Item/R1InventorySubsystem.h"

void UR1GameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Retry)
	{
		Button_Retry->OnClicked.AddDynamic(this, &UR1GameOverWidget::OnRetryClicked);
	}

	if (Button_Exit)
	{
		Button_Exit->OnClicked.AddDynamic(this, &UR1GameOverWidget::OnExitClicked);
	}
}

void UR1GameOverWidget::OnRetryClicked()
{
	if (UWorld* World = GetWorld())
	{
		// 1. 세이브 데이터 삭제
		if (UR1SaveSystem* SaveSystem = World->GetGameInstance()->GetSubsystem<UR1SaveSystem>())
		{
			SaveSystem->DeleteSavedRun();
		}

		// 2. 인벤토리 초기화 (데이터 상으로는 Clear 후 기본 아이템 지급)
		if (UR1InventorySubsystem* InventorySubsystem = World->GetSubsystem<UR1InventorySubsystem>())
		{
			InventorySubsystem->ClearInventory();
			// InventorySubsystem->AddDefaultItem(); // GameMode::InitGame에서 레벨 로드 시 어차피 호출됨
		}

		// 3. 레벨 재시작 (OpenLevel을 하면 GameMode::InitGame부터 다시 시작하여 맵이 새로 생성되고 플레이어 스탯도 초기화됨)
		FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(World);
		UGameplayStatics::OpenLevel(World, FName(*CurrentLevelName));
	}
}

void UR1GameOverWidget::OnExitClicked()
{
	if (UWorld* World = GetWorld())
	{
		// 사망 후 타이틀로 나갈 때도 현재 런 세이브 데이터 삭제
		if (UR1SaveSystem* SaveSystem = World->GetGameInstance()->GetSubsystem<UR1SaveSystem>())
		{
			SaveSystem->DeleteSavedRun();
		}

		UGameplayStatics::OpenLevel(World, TEXT("TitleMap"));
	}
}




#include "UI/Progression/R1MetaUpgradeWidget.h"
#include "UI/Progression/R1MetaUpgradeSlotWidget.h"
#include "Components/TextBlock.h"
#include "Components/PanelWidget.h"
#include "Components/Button.h"
#include "System/R1SaveSystem.h"
#include "System/R1MetaSaveGame.h"
#include "DataTable/R1MetaUpgradeData.h"
#include "Player/R1MainMenuController.h"

void UR1MetaUpgradeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);

	RefreshUI();

	if (Button_Back)
	{
		Button_Back->OnClicked.AddDynamic(this, &UR1MetaUpgradeWidget::OnButtonBackClicked);
	}
}

FReply UR1MetaUpgradeWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		if (AR1MainMenuController* MenuPC = Cast<AR1MainMenuController>(GetOwningPlayer()))
		{
			MenuPC->GoBack(); // 컨트롤러의 만능 뒤로가기!
			return FReply::Handled();
		}
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UR1MetaUpgradeWidget::RefreshUI()
{
	if (!Panel_SkillList || !SlotWidgetClass || !MetaDataTable) return;

	UR1SaveSystem* SaveSystem = GetGameInstance()->GetSubsystem<UR1SaveSystem>();
	if (!SaveSystem) return;

	// 1. 금고에서 현재 데이터 가져오기
	UR1MetaSaveGame* MetaSave = SaveSystem->LoadMetaProgression();
	if (!MetaSave) return;

	// 2. 남은 포인트 UI 갱신
	if (Text_AvailablePoints)
	{
		Text_AvailablePoints->SetText(FText::AsNumber(MetaSave->AvailableSkillPoints));
	}

	// 3. 기존 슬롯들 다 지우기
	Panel_SkillList->ClearChildren();

	// 4. 데이터 테이블을 순회하며 슬롯 생성
	TArray<FR1MetaUpgradeData*> AllUpgrades;
	MetaDataTable->GetAllRows<FR1MetaUpgradeData>(TEXT("MetaUI"), AllUpgrades);

	bool bHasPoints = (MetaSave->AvailableSkillPoints > 0);

	for (FR1MetaUpgradeData* UpgradeData : AllUpgrades)
	{
		if (!UpgradeData) continue;

		// 유저가 이 스킬을 몇 렙 찍었는지 확인 (없으면 0렙)
		int32 CurrentLevel = 0;
		if (int32* FoundLevel = MetaSave->InvestedUpgrades.Find(UpgradeData->UpgradeTag))
		{
			CurrentLevel = *FoundLevel;
		}

		// 조건: 플레이어의 누적 메타 레벨이 스킬의 요구 레벨 이상일 때만 화면에 표시
		if (MetaSave->PlayerMetaLevel >= UpgradeData->RequiredPlayerLevel)
		{
			UR1MetaUpgradeSlotWidget* NewSlot = CreateWidget<UR1MetaUpgradeSlotWidget>(this, SlotWidgetClass);
			if (NewSlot)
			{
				// 슬롯에 데이터 주입
				NewSlot->InitSlot(UpgradeData->UpgradeTag, UpgradeData->UpgradeName, CurrentLevel, UpgradeData->MaxLevel, bHasPoints);

				// 슬롯의 클릭 이벤트 구독
				NewSlot->OnUpgradeButtonClicked.AddDynamic(this, &UR1MetaUpgradeWidget::HandleUpgradeRequest);

				Panel_SkillList->AddChild(NewSlot);
			}
		}
	}
}

void UR1MetaUpgradeWidget::HandleUpgradeRequest(FGameplayTag RequestedTag)
{
	UR1SaveSystem* SaveSystem = GetGameInstance()->GetSubsystem<UR1SaveSystem>();
	if (!SaveSystem)
	{
		return;
	}
	UR1MetaSaveGame* MetaSave = SaveSystem->LoadMetaProgression();
	if (!MetaSave || MetaSave->AvailableSkillPoints <= 0)
	{
		return;
	}
	// 데이터 테이블에서 정보 찾기
	FR1MetaUpgradeData* UpgradeData = nullptr;
	TArray<FR1MetaUpgradeData*> AllUpgrades;
	MetaDataTable->GetAllRows<FR1MetaUpgradeData>(TEXT("MetaUI"), AllUpgrades);

	for (auto* Data : AllUpgrades)
	{
		if (Data && Data->UpgradeTag == RequestedTag)
		{
			UpgradeData = Data;
			break;
		}
	}

	if (!UpgradeData)
	{
		return;
	}

	// 현재 레벨 확인
	int32 CurrentLevel = MetaSave->InvestedUpgrades.Contains(RequestedTag) ? MetaSave->InvestedUpgrades[RequestedTag] : 0;

	// 만렙인지 확인
	if (CurrentLevel < UpgradeData->MaxLevel)
	{
		// 🌟 대망의 레벨업 & 결제 처리!
		MetaSave->AvailableSkillPoints -= 1;
		MetaSave->InvestedUpgrades.Add(RequestedTag, CurrentLevel + 1);

		// 금고 덮어쓰기
		SaveSystem->SaveMetaProgression(MetaSave);

		// 화면 즉시 갱신 (포인트 줄어들고 레벨 올라간 거 반영)
		RefreshUI();

		UE_LOG(LogTemp, Warning, TEXT("[MetaUI] %s 레벨업 성공! (현재 %d 렙)"), *RequestedTag.ToString(), CurrentLevel + 1);
	}
}

void UR1MetaUpgradeWidget::OnButtonBackClicked()
{
	if (AR1MainMenuController* MenuPC = Cast<AR1MainMenuController>(GetOwningPlayer()))
	{
		MenuPC->GoBack();
	}
}

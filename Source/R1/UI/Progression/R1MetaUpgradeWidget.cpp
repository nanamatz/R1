


#include "UI/Progression/R1MetaUpgradeWidget.h"
#include "UI/Progression/R1MetaUpgradeSlotWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
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

	if (Text_MetaPlayerLevel)
	{
		Text_MetaPlayerLevel->SetText(FText::AsNumber(MetaSave->PlayerMetaLevel));
	}

	// 3. 기존 슬롯들 다 지우기
	Panel_SkillList->ClearChildren();

	// 4. 데이터 테이블을 순회하며 슬롯 생성
	TArray<FR1MetaUpgradeData*> AllUpgrades;
	MetaDataTable->GetAllRows<FR1MetaUpgradeData>(TEXT("MetaUI"), AllUpgrades);

	bool bHasPoints = (MetaSave->AvailableSkillPoints > 0);
	bool bHasLocked = false;

	int32 CurrentSlotIndex = 0;

	for (FR1MetaUpgradeData* UpgradeData : AllUpgrades)
	{
		if (!UpgradeData) continue;

		bHasLocked = (MetaSave->PlayerMetaLevel < UpgradeData->RequiredPlayerLevel);

		//어차피 player Meta Level이 요구 레벨 이상일 때만 필요한 정보이므로 제어문 안으로 넣어줌.
		int32 CurrentLevel = 0;
		if (int32* FoundLevel = MetaSave->InvestedUpgrades.Find(UpgradeData->UpgradeTag))
		{
			CurrentLevel = *FoundLevel;
		}

		UR1MetaUpgradeSlotWidget* NewSlot = CreateWidget<UR1MetaUpgradeSlotWidget>(this, SlotWidgetClass);
		if (NewSlot)
		{
			NewSlot->InitSlot(UpgradeData->UpgradeTag, UpgradeData->UpgradeName, CurrentLevel, UpgradeData->MaxLevel,UpgradeData->RequiredPlayerLevel, UpgradeData->UpgradeIcon,bHasPoints,bHasLocked);
			NewSlot->OnUpgradeButtonClicked.AddDynamic(this, &UR1MetaUpgradeWidget::HandleUpgradeRequest);

			// 🌟 1. 격자 패널에 위젯을 추가하고, 그 반환값을 UniformGridSlot으로 받습니다.
			UUniformGridSlot* GridSlot = Panel_SkillList->AddChildToUniformGrid(NewSlot);

			if (GridSlot)
			{
				// 🌟 2. 몫(Row)과 나머지(Column)를 이용한 마법의 격자 공식!
				int32 Row = CurrentSlotIndex / GridColumn;
				int32 Col = CurrentSlotIndex % GridColumn;

				GridSlot->SetRow(Row);
				GridSlot->SetColumn(Col);

				// (선택) 슬롯이 격자 칸에 꽉 차게 예쁘게 들어가도록 정렬 설정
				GridSlot->SetHorizontalAlignment(HAlign_Fill);
				GridSlot->SetVerticalAlignment(VAlign_Fill);
			}

			// 다음 스킬 배치를 위해 인덱스 증가
			CurrentSlotIndex++;
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

	if (CurrentLevel < UpgradeData->MaxLevel)
	{
		MetaSave->AvailableSkillPoints -= 1;
		MetaSave->InvestedUpgrades.Add(RequestedTag, CurrentLevel + 1);

		SaveSystem->SaveMetaProgression(MetaSave);

		RefreshUI();
	}
}

void UR1MetaUpgradeWidget::OnButtonBackClicked()
{
	if (AR1MainMenuController* MenuPC = Cast<AR1MainMenuController>(GetOwningPlayer()))
	{
		MenuPC->GoBack();
	}
}

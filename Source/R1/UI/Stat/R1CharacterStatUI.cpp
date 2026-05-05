#include "UI/Stat/R1CharacterStatUI.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Player/R1PlayerState.h"
#include "Player/R1RunUpgradeComponent.h"
#include "AbilitySystem/Attribute/PlayerAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "DataTable/R1StatUpgradeData.h"
#include "Library/R1StatFormattingLibrary.h"
#include "UI/Stat/R1StatUpgradeRow.h"
#include "UI/Stat/R1StatDetailRow.h"
#include "Engine/DataTable.h"

void UR1CharacterStatUI::NativeConstruct()
{
	Super::NativeConstruct();

	AR1PlayerState* PS = Cast<AR1PlayerState>(GetOwningPlayerState());
	if (PS)
	{
		if (UR1RunUpgradeComponent* RunUpgradeComp = PS->GetRunUpgradeComponent())
		{
			RunUpgradeComp->OnAvailablePointsChanged.AddUniqueDynamic(this, &UR1CharacterStatUI::HandleAvailablePointsChanged);
			RunUpgradeComp->OnInvestmentHistoryChanged.AddUniqueDynamic(this, &UR1CharacterStatUI::HandleInvestmentHistoryChanged);
		}
	}

	RefreshUI();
}

void UR1CharacterStatUI::RefreshUI()
{
	AR1PlayerState* PS = Cast<AR1PlayerState>(GetOwningPlayerState());
	if (!PS) return;

	UR1RunUpgradeComponent* RunUpgradeComp = PS->GetRunUpgradeComponent();
	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	const UPlayerAttributeSet* PlayerAS = PS->GetPlayerAttributeSet();

	if (!RunUpgradeComp || !ASC || !PlayerAS) return;

	// Update Available Points
	if (Text_RemainPointAmount)
	{
		Text_RemainPointAmount->SetText(FText::AsNumber(RunUpgradeComp->GetAvailablePoints()));
	}

	// Update Level/Exp/Class text
	if (Text_ClassName)
	{
		FString CharacterClassName = TEXT("Player");
		if (APawn* OwningPawn = GetOwningPlayerPawn())
		{
			CharacterClassName = OwningPawn->GetClass()->GetName();
			CharacterClassName.RemoveFromEnd(TEXT("_C"));
		}
		Text_ClassName->SetText(FText::FromString(CharacterClassName)); 
	}

	if (Text_CurrentExp && Text_ExpToLevelUp)
	{
		Text_CurrentExp->SetText(FText::AsNumber(FMath::FloorToInt(PlayerAS->GetExp())));
		Text_ExpToLevelUp->SetText(FText::AsNumber(FMath::FloorToInt(PlayerAS->GetMaxExp())));
	}

	// Refresh Upgrade List
	if (ScrollBox_UpgradeList && StatUpgradeDataTable)
	{
		TArray<UR1StatUpgradeRow*> Rows;
		for (UWidget* Child : ScrollBox_UpgradeList->GetAllChildren())
		{
			if (UR1StatUpgradeRow* Row = Cast<UR1StatUpgradeRow>(Child))
			{
				Rows.Add(Row);
			}
		}

		TArray<FR1StatUpgradeData*> AllUpgradeData;
		StatUpgradeDataTable->GetAllRows<FR1StatUpgradeData>(TEXT(""), AllUpgradeData);

		for (UR1StatUpgradeRow* Row : Rows)
		{
			FText RowName = Row->GetAttributeName();
			for (FR1StatUpgradeData* Data : AllUpgradeData)
			{
				if (Data->StatName.EqualTo(RowName))
				{
					int32 Count = RunUpgradeComp->GetInvestmentCount(Data->StatTag);
					Row->InjectData(Count);
					Row->SetStatTag(Data->StatTag);
					
					// Bind upgrade request
					Row->OnUpgradeRowClicked.RemoveAll(this);
					Row->OnUpgradeRowClicked.AddUniqueDynamic(this, &UR1CharacterStatUI::OnUpgradeStatClicked);
					break;
				}
			}
		}
	}

	// Refresh Detail List
	if (ScrollBox_DetailList && StatUpgradeDataTable)
	{
		TArray<UR1StatDetailRow*> Rows;
		for (UWidget* Child : ScrollBox_DetailList->GetAllChildren())
		{
			if (UR1StatDetailRow* Row = Cast<UR1StatDetailRow>(Child))
			{
				Rows.Add(Row);
			}
		}

		TArray<FR1StatUpgradeData*> AllUpgradeData;
		StatUpgradeDataTable->GetAllRows<FR1StatUpgradeData>(TEXT(""), AllUpgradeData);

		for (UR1StatDetailRow* Row : Rows)
		{
			FText RowName = Row->GetAttributeName();
			for (FR1StatUpgradeData* Data : AllUpgradeData)
			{
				if (Data->StatName.EqualTo(RowName))
				{
					FText FormattedValue;
					if (Data->DisplayType == ER1StatDisplayType::Range)
					{
						FormattedValue = UR1StatFormattingLibrary::GetWeaponDamageRangeText(ASC);
					}
					else
					{
						float AttrValue = ASC->GetNumericAttribute(Data->Attribute);
						FormattedValue = UR1StatFormattingLibrary::FormatStatValue(AttrValue, Data->DisplayType);
					}
					Row->InjectData(FormattedValue);
					break;
				}
			}
		}
	}
}

void UR1CharacterStatUI::HandleAvailablePointsChanged(int32 NewPoints)
{
	if (Text_RemainPointAmount)
	{
		Text_RemainPointAmount->SetText(FText::AsNumber(NewPoints));
	}
}

void UR1CharacterStatUI::HandleInvestmentHistoryChanged(FGameplayTag StatTag, int32 NewCount)
{
	RefreshUI();
}

void UR1CharacterStatUI::OnUpgradeStatClicked(FGameplayTag StatTag)
{
	AR1PlayerState* PS = Cast<AR1PlayerState>(GetOwningPlayerState());
	if (PS)
	{
		if (UR1RunUpgradeComponent* RunUpgradeComp = PS->GetRunUpgradeComponent())
		{
			RunUpgradeComp->UpgradeStat(StatTag);
		}
	}
}

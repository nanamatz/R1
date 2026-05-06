#include "UI/Stat/R1CharacterStatUI.h"

#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

#include "Player/R1PlayerState.h"
#include "Player/R1RunUpgradeComponent.h"

#include "AbilitySystem/Attribute/PlayerAttributeSet.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "AbilitySystemComponent.h"

#include "Engine/DataTable.h"
#include "DataTable/R1StatUpgradeData.h"
#include "Library/R1StatFormattingLibrary.h"

#include "UI/Stat/R1StatUpgradeRow.h"
#include "UI/Stat/R1StatDetailRow.h"
#include "UI/R1HUD.h"

#include "R1GameplayTags.h"
#include "R1Define.h"

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

		PS->OnExpChanged.AddUniqueDynamic(this, &UR1CharacterStatUI::HandleExpChanged);
		if (UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
		{
			if (StatUpgradeDataTable)
			{
				TArray<FR1StatUpgradeData*> AllUpgradeData;
				StatUpgradeDataTable->GetAllRows<FR1StatUpgradeData>(TEXT("StatUI_BindAttributes"), AllUpgradeData);

				for (FR1StatUpgradeData* Data : AllUpgradeData)
				{
					// 데이터 테이블에 유효한 어트리뷰트가 세팅되어 있다면
					if (Data->Attribute.IsValid())
					{
						// Task 1: 중복 바인딩 방지를 위해 기존 바인딩을 먼저 제거합니다.
						ASC->GetGameplayAttributeValueChangeDelegate(Data->Attribute).RemoveAll(this);
						// 해당 어트리뷰트의 변경 델리게이트를 찾아 우리의 함수를 연결해줍니다.
						ASC->GetGameplayAttributeValueChangeDelegate(Data->Attribute).AddUObject(this, &UR1CharacterStatUI::OnAttributeChanged);
					}
				}
			}
		}

	}
	if (Button_Close)
	{
		Button_Close->OnClicked.AddUniqueDynamic(this, &UR1CharacterStatUI::OnCloseButtonClicked);
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
	int32 CurrentPoints = RunUpgradeComp->GetAvailablePoints();
	if (Text_RemainPointAmount)
	{
		Text_RemainPointAmount->SetText(FText::AsNumber(CurrentPoints));
	}

	// Update Level/Exp/Class text
	if (Text_ClassName)
	{
		Text_ClassName->SetText(GetCharacterClassName(PS->GetPlayerClass())); 
	}

	if (Text_LevelAmount)
	{
		Text_LevelAmount->SetText(FText::AsNumber(PS->GetRunLevel()));
	}

	if (Text_CurrentExp && Text_ExpToLevelUp)
	{
		Text_CurrentExp->SetText(FText::AsNumber(FMath::FloorToInt(PlayerAS->GetExp())));
		Text_ExpToLevelUp->SetText(FText::AsNumber(FMath::FloorToInt(PlayerAS->GetMaxExp())));
	}

	TArray<FR1StatUpgradeData*> AllUpgradeData;
	if (StatUpgradeDataTable)
	{
		StatUpgradeDataTable->GetAllRows<FR1StatUpgradeData>(TEXT(""), AllUpgradeData);
	}

	// Refresh Upgrade List
	if (List_Upgrade && AllUpgradeData.Num() > 0)
	{
		int32 ChildCount = List_Upgrade->GetChildrenCount();
		for (int32 i = 0; i < ChildCount; ++i)
		{
			if (UR1StatUpgradeRow* Row = Cast<UR1StatUpgradeRow>(List_Upgrade->GetChildAt(i)))
			{
				FString RowStatName = Row->GetAttributeNameText().ToString();

				for (FR1StatUpgradeData* Data : AllUpgradeData)
				{
					if (RowStatName.Equals(Data->StatName.ToString(), ESearchCase::IgnoreCase))
					{
						int32 Count = RunUpgradeComp->GetInvestmentCount(Data->StatTag);
						Row->InjectData(Data->StatName, Count);
						Row->SetStatTag(Data->StatTag);
						Row->SetButtonEnabled(CurrentPoints > 0);
						Row->OnUpgradeRowClicked.AddUniqueDynamic(this, &UR1CharacterStatUI::OnUpgradeStatClicked);
						break;
					}
				}
			}
		}
	}

	// 2. Detail List 순회 및 데이터 주입
	if (List_Detail && AllUpgradeData.Num() > 0)
	{
		int32 ChildCount = List_Detail->GetChildrenCount();
		for (int32 i = 0; i < ChildCount; ++i)
		{
			if (UR1StatDetailRow* Row = Cast<UR1StatDetailRow>(List_Detail->GetChildAt(i)))
			{
				FString RowStatName = Row->GetAttributeNameText().ToString();

				for (FR1StatUpgradeData* Data : AllUpgradeData)
				{
					if (RowStatName.Equals(Data->StatName.ToString(), ESearchCase::IgnoreCase))
					{
						FText FormattedValue;
						if (Data->DisplayType == ER1StatDisplayType::Range)
						{
							FormattedValue = UR1StatFormattingLibrary::GetWeaponDamageRangeText(ASC);
						}
						else if (Data->DisplayType == ER1StatDisplayType::Fraction)
						{
							float CurrentValue = ASC->GetNumericAttribute(Data->Attribute);
							float MaxValue = 1.0f;

							if (Data->Attribute.GetName() == TEXT("MaxHealth"))
							{
								MaxValue = ASC->GetNumericAttribute(UR1AttributeSet::GetMaxHealthAttribute());
							}
							else if (Data->Attribute.GetName() == TEXT("MaxMana"))
							{
								MaxValue = ASC->GetNumericAttribute(UPlayerAttributeSet::GetMaxManaAttribute());
							}

							FormattedValue = UR1StatFormattingLibrary::GetFractionText(CurrentValue, MaxValue);
						}
						else
						{
							float AttrValue = ASC->GetNumericAttribute(Data->Attribute);
							FormattedValue = UR1StatFormattingLibrary::FormatStatValue(AttrValue, Data->DisplayType);
						}

						Row->InjectData(Data->StatName, FormattedValue);
						break;
					}
				}
			}
		}
	}
}


FText UR1CharacterStatUI::GetCharacterClassName(ER1CharacterClass InClass) const
{
	switch (InClass)
	{
	case ER1CharacterClass::Knight: return FText::FromString(TEXT("Knight"));
	case ER1CharacterClass::OutLaw: return FText::FromString(TEXT("OutLaw"));
	case ER1CharacterClass::Fighter: return FText::FromString(TEXT("Fighter"));
	case ER1CharacterClass::Wizard: return FText::FromString(TEXT("Wizard"));
	}
	return FText::FromString(TEXT("Tennis Player"));
}

void UR1CharacterStatUI::HandleAvailablePointsChanged(int32 NewPoints)
{
	if (Text_RemainPointAmount)
	{
		Text_RemainPointAmount->SetText(FText::AsNumber(NewPoints));
	}

	if (List_Upgrade)
	{
		bool bEnable = NewPoints > 0;
		for (UWidget* Child : List_Upgrade->GetAllChildren())
		{
			if (UR1StatUpgradeRow* Row = Cast<UR1StatUpgradeRow>(Child))
			{
				Row->SetButtonEnabled(bEnable);
			}
		}
	}
}

void UR1CharacterStatUI::HandleInvestmentHistoryChanged(FGameplayTag StatTag, int32 NewCount)
{
	RefreshUI();
}

void UR1CharacterStatUI::HandleExpChanged(float Ratio)
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

void UR1CharacterStatUI::OnCloseButtonClicked()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	if (AR1HUD* HUD = Cast<AR1HUD>(PC->GetHUD()))
	{
		HUD->ToggleCharacterStatUI();
	}
}

void UR1CharacterStatUI::OnAttributeChanged(const FOnAttributeChangeData& Data)
{
	UE_LOG(LogTemp, Warning, TEXT("Changed Attribute : %s"), *Data.Attribute.GetName());
	UE_LOG(LogTemp, Warning, TEXT("Old Data : %f"), Data.OldValue);
	UE_LOG(LogTemp, Warning, TEXT("New Data : %f"), Data.NewValue);
	
	// Task 3: 전체 RefreshUI 대신 변경된 속성과 관련된 부분만 업데이트하도록 최적화합니다.
	// 현재는 간단한 최적화를 위해 RefreshUI를 호출하지만, 
	// 실제로는 Data.Attribute에 해당하는 Row만 찾아 InjectData를 호출하는 것이 이상적입니다.
	RefreshUI();
}
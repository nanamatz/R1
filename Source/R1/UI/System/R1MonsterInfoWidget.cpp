
#include "UI/System/R1MonsterInfoWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "System/R1LocalizationSubsystem.h"

void UR1MonsterInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	if (GI)
	{
		if (UR1LocalizationSubsystem* LocSub = GI->GetSubsystem<UR1LocalizationSubsystem>())
		{
			LocSub->OnLanguageChanged.AddUObject(this, &UR1MonsterInfoWidget::RefreshLocalization);
		}
	}
}

void UR1MonsterInfoWidget::NativeDestruct()
{
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	if (GI)
	{
		if (UR1LocalizationSubsystem* LocSub = GI->GetSubsystem<UR1LocalizationSubsystem>())
		{
			LocSub->OnLanguageChanged.RemoveAll(this);
		}
	}
	Super::NativeDestruct();
}

void UR1MonsterInfoWidget::UpdateMonsterInfo(FName CharacterRowName, float HpRatio)
{
	CachedMonsterRowName = CharacterRowName;

	if (HpBar)
	{
		HpBar->SetPercent(HpRatio);
	}

	if (MonsterName)
	{
		UWorld* World = GetWorld();
		UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
		UR1LocalizationSubsystem* LocSub = GI ? GI->GetSubsystem<UR1LocalizationSubsystem>() : nullptr;

		FText DisplayName = LocSub ? LocSub->GetText(CharacterRowName) : FText::FromName(CharacterRowName);
		MonsterName->SetText(DisplayName);
	}
}

void UR1MonsterInfoWidget::RefreshLocalization()
{
	if (MonsterName && !CachedMonsterRowName.IsNone())
	{
		UWorld* World = GetWorld();
		UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
		UR1LocalizationSubsystem* LocSub = GI ? GI->GetSubsystem<UR1LocalizationSubsystem>() : nullptr;

		if (LocSub)
		{
			MonsterName->SetText(LocSub->GetText(CachedMonsterRowName));
		}
	}
}

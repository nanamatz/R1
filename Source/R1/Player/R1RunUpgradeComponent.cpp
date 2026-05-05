#include "Player/R1RunUpgradeComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "DataTable/R1StatUpgradeData.h"
#include "R1GameplayTags.h"
#include "R1LogChannels.h"

UR1RunUpgradeComponent::UR1RunUpgradeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UR1RunUpgradeComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UR1RunUpgradeComponent::AddPoints(int32 Amount)
{
	AvailablePoints += Amount;
	OnAvailablePointsChanged.Broadcast(AvailablePoints);
}

void UR1RunUpgradeComponent::UpgradeStat(FGameplayTag StatTag)
{
	if (AvailablePoints <= 0)
	{
		UE_LOG(LogR1, Warning, TEXT("UR1RunUpgradeComponent::UpgradeStat: Not enough points to upgrade %s"), *StatTag.ToString());
		return;
	}

	if (!StatUpgradeDataTable)
	{
		UE_LOG(LogR1, Error, TEXT("UR1RunUpgradeComponent::UpgradeStat: StatUpgradeDataTable is null!"));
		return;
	}

	static const FString ContextString(TEXT("UR1RunUpgradeComponent::UpgradeStat"));
	TArray<FR1StatUpgradeData*> AllRows;
	StatUpgradeDataTable->GetAllRows<FR1StatUpgradeData>(ContextString, AllRows);

	bool bFound = false;
	for (FR1StatUpgradeData* Row : AllRows)
	{
		if (Row && Row->StatTag == StatTag)
		{
			bFound = true;
			break;
		}
	}

	if (!bFound)
	{
		UE_LOG(LogR1, Warning, TEXT("UR1RunUpgradeComponent::UpgradeStat: Could not find StatTag %s in DataTable"), *StatTag.ToString());
		return;
	}

	AvailablePoints--;
	InvestmentHistory.FindOrAdd(StatTag)++;

	OnAvailablePointsChanged.Broadcast(AvailablePoints);
	OnInvestmentHistoryChanged.Broadcast(StatTag, InvestmentHistory[StatTag]);

	ApplyRunUpgradeEffect();
}

void UR1RunUpgradeComponent::Reset()
{
	AvailablePoints = 0;
	
	TArray<FGameplayTag> InvestedTags;
	InvestmentHistory.GetKeys(InvestedTags);

	InvestmentHistory.Empty();
	
	OnAvailablePointsChanged.Broadcast(AvailablePoints);
	for (const FGameplayTag& StatTag : InvestedTags)
	{
		OnInvestmentHistoryChanged.Broadcast(StatTag, 0);
	}
	
	if (RunUpgradeGEHandle.IsValid())
	{
		AActor* Owner = GetOwner();
		UAbilitySystemComponent* ASC = nullptr;
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner))
		{
			ASC = ASI->GetAbilitySystemComponent();
		}
		else if (Owner)
		{
			ASC = Owner->FindComponentByClass<UAbilitySystemComponent>();
		}

		if (ASC)
		{
			ASC->RemoveActiveGameplayEffect(RunUpgradeGEHandle);
		}
		RunUpgradeGEHandle = FActiveGameplayEffectHandle();
	}
}

int32 UR1RunUpgradeComponent::GetInvestmentCount(FGameplayTag StatTag) const
{
	return InvestmentHistory.Contains(StatTag) ? InvestmentHistory[StatTag] : 0;
}

void UR1RunUpgradeComponent::ApplyRunUpgradeEffect()
{
	if (!RunUpgradeGEClass)
	{
		UE_LOG(LogR1, Error, TEXT("UR1RunUpgradeComponent::ApplyRunUpgradeEffect: RunUpgradeGEClass is null!"));
		return;
	}
	if (!StatUpgradeDataTable)
	{
		UE_LOG(LogR1, Error, TEXT("UR1RunUpgradeComponent::ApplyRunUpgradeEffect: StatUpgradeDataTable is null!"));
		return;
	}

	AActor* Owner = GetOwner();
	UAbilitySystemComponent* ASC = nullptr;
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner))
	{
		ASC = ASI->GetAbilitySystemComponent();
	}
	else if (Owner)
	{
		ASC = Owner->FindComponentByClass<UAbilitySystemComponent>();
	}

	if (!ASC)
	{
		UE_LOG(LogR1, Error, TEXT("UR1RunUpgradeComponent::ApplyRunUpgradeEffect: Could not find ASC on owner!"));
		return;
	}

	// Remove old effect if it exists
	if (RunUpgradeGEHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(RunUpgradeGEHandle);
		RunUpgradeGEHandle = FActiveGameplayEffectHandle();
	}

	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(RunUpgradeGEClass, 1.0f, EffectContext);
	if (SpecHandle.IsValid())
	{
		static const FString ContextString(TEXT("UR1RunUpgradeComponent::ApplyRunUpgradeEffect"));
		TArray<FR1StatUpgradeData*> AllRows;
		StatUpgradeDataTable->GetAllRows<FR1StatUpgradeData>(ContextString, AllRows);

		// Initialize all stat magnitudes to 0.0f to prevent GAS errors for uninvested stats
		for (FR1StatUpgradeData* Row : AllRows)
		{
			if (Row)
			{
				SpecHandle.Data.Get()->SetSetByCallerMagnitude(Row->StatTag, 0.0f);
			}
		}

		for (const auto& Pair : InvestmentHistory)
		{
			FGameplayTag StatTag = Pair.Key;
			int32 InvestmentCount = Pair.Value;

			FR1StatUpgradeData* FoundData = nullptr;
			for (FR1StatUpgradeData* Row : AllRows)
			{
				if (Row && Row->StatTag == StatTag)
				{
					FoundData = Row;
					break;
				}
			}

			if (FoundData)
			{
				float TotalBonus = InvestmentCount * FoundData->IncreaseAmount;
				SpecHandle.Data.Get()->SetSetByCallerMagnitude(StatTag, TotalBonus);
			}
			else
			{
				UE_LOG(LogR1, Warning, TEXT("UR1RunUpgradeComponent::ApplyRunUpgradeEffect: Could not find data for tag %s in DataTable"), *StatTag.ToString());
			}
		}

		RunUpgradeGEHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

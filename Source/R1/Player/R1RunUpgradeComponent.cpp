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
	CacheDataTable();
}

void UR1RunUpgradeComponent::CacheDataTable()
{
	if (!StatUpgradeDataTable)
	{
		return;
	}

	CachedStatUpgradeData.Empty();

	static const FString ContextString(TEXT("UR1RunUpgradeComponent::CacheDataTable"));
	TArray<FR1StatUpgradeData*> AllRows;
	StatUpgradeDataTable->GetAllRows<FR1StatUpgradeData>(ContextString, AllRows);

	for (FR1StatUpgradeData* Row : AllRows)
	{
		if (Row)
		{
			CachedStatUpgradeData.Add(Row->StatTag, *Row);
		}
	}
}

UAbilitySystemComponent* UR1RunUpgradeComponent::GetAbilitySystemComponent() const
{
	AActor* Owner = GetOwner();
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner))
	{
		return ASI->GetAbilitySystemComponent();
	}
	else if (Owner)
	{
		return Owner->FindComponentByClass<UAbilitySystemComponent>();
	}

	return nullptr;
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

	if (!CachedStatUpgradeData.Contains(StatTag))
	{
		UE_LOG(LogR1, Warning, TEXT("UR1RunUpgradeComponent::UpgradeStat: Could not find StatTag %s in DataTable"), *StatTag.ToString());
		return;
	}

	AvailablePoints--;
	InvestmentHistory.FindOrAdd(StatTag)++;

	ApplyRunUpgradeEffect();

	OnAvailablePointsChanged.Broadcast(AvailablePoints);
	OnInvestmentHistoryChanged.Broadcast(StatTag, InvestmentHistory[StatTag]);

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
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		{
			ASC->RemoveActiveGameplayEffect(RunUpgradeGEHandle);
		}
		RunUpgradeGEHandle = FActiveGameplayEffectHandle();
	}
}

void UR1RunUpgradeComponent::LoadUpgradeData(int32 InAvailablePoints, const TMap<FGameplayTag, int32>& InHistory)
{
	AvailablePoints = InAvailablePoints;
	InvestmentHistory = InHistory;

	ApplyRunUpgradeEffect();

	OnAvailablePointsChanged.Broadcast(AvailablePoints);
	for (const auto& Pair : InvestmentHistory)
	{
		OnInvestmentHistoryChanged.Broadcast(Pair.Key, Pair.Value);
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

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
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
		// Initialize all stat magnitudes to 0.0f to prevent GAS errors for uninvested stats
		for (const auto& Pair : CachedStatUpgradeData)
		{
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(Pair.Key, 0.0f);
		}

		for (const auto& Pair : InvestmentHistory)
		{
			FGameplayTag StatTag = Pair.Key;
			int32 InvestmentCount = Pair.Value;

			if (FR1StatUpgradeData* FoundDataPtr = CachedStatUpgradeData.Find(StatTag))
			{
				float TotalBonus = InvestmentCount * FoundDataPtr->IncreaseAmount;
				SpecHandle.Data.Get()->SetSetByCallerMagnitude(StatTag, TotalBonus);
			}
			else
			{
				UE_LOG(LogR1, Warning, TEXT("UR1RunUpgradeComponent::ApplyRunUpgradeEffect: Could not find data for tag %s in CachedStatUpgradeData"), *StatTag.ToString());
			}
		}

		RunUpgradeGEHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

#include "System/R1RoomStreamingSubsystem.h"

#include "Data/R1RoomDefinitionData.h"
#include "Engine/AssetManager.h"
#include "Engine/LevelStreamingDynamic.h"
#include "System/R1AssetManager.h"

void UR1RoomStreamingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RoomStates.Reset();
}

void UR1RoomStreamingSubsystem::Deinitialize()
{
	RoomStates.Reset();
	Super::Deinitialize();
}

void UR1RoomStreamingSubsystem::SetRuntimeBudget(const FR1RuntimeBudget& InBudget)
{
	Budget = InBudget;
	TrimPreloadIfNeeded();
}

FR1RuntimeBudget UR1RoomStreamingSubsystem::GetRuntimeBudget() const
{
	return Budget;
}

void UR1RoomStreamingSubsystem::QueuePreloadRooms(const TArray<UR1RoomDefinitionData*>& CandidateRooms)
{
	for (UR1RoomDefinitionData* Candidate : CandidateRooms)
	{
		BeginPreload(Candidate);
	}

	TrimPreloadIfNeeded();
}

void UR1RoomStreamingSubsystem::MarkRoomGameplayReady(UR1RoomDefinitionData* RoomDefinition)
{
	if (RoomDefinition == nullptr)
	{
		return;
	}

	const FName RoomKey = MakeRoomKey(RoomDefinition);
	if (FR1RoomRuntimeState* State = RoomStates.Find(RoomKey))
	{
		State->ThermalState = ER1RoomThermalState::Hot;
		State->LastTouchedTime = FPlatformTime::Seconds();
		OnRoomBecameHot.Broadcast(RoomKey);
	}
}

ER1RoomThermalState UR1RoomStreamingSubsystem::GetRoomState(UR1RoomDefinitionData* RoomDefinition) const
{
	if (RoomDefinition == nullptr)
	{
		return ER1RoomThermalState::Cold;
	}

	const FName RoomKey = MakeRoomKey(RoomDefinition);
	if (const FR1RoomRuntimeState* State = RoomStates.Find(RoomKey))
	{
		return State->ThermalState;
	}

	return ER1RoomThermalState::Cold;
}

bool UR1RoomStreamingSubsystem::CanOpenDoorImmediately(UR1RoomDefinitionData* RoomDefinition) const
{
	const ER1RoomThermalState State = GetRoomState(RoomDefinition);
	return State == ER1RoomThermalState::Hot;
}

void UR1RoomStreamingSubsystem::TickRoomCachePolicy()
{
	const double Now = FPlatformTime::Seconds();

	for (auto& Pair : RoomStates)
	{
		FR1RoomRuntimeState& State = Pair.Value;
		if (State.ThermalState == ER1RoomThermalState::Cold)
		{
			continue;
		}

		if (Now - State.LastTouchedTime > Budget.UnloadGraceSeconds)
		{
			State.ThermalState = ER1RoomThermalState::Cold;
			State.StreamingLevel = nullptr;
		}
	}

	TrimPreloadIfNeeded();
}

void UR1RoomStreamingSubsystem::BeginPreload(UR1RoomDefinitionData* RoomDefinition)
{
	if (RoomDefinition == nullptr)
	{
		return;
	}

	const FName RoomKey = MakeRoomKey(RoomDefinition);
	FR1RoomRuntimeState& State = RoomStates.FindOrAdd(RoomKey);
	State.LastTouchedTime = FPlatformTime::Seconds();

	if (State.ThermalState == ER1RoomThermalState::Hot || State.ThermalState == ER1RoomThermalState::Warm)
	{
		return;
	}

	State.ThermalState = ER1RoomThermalState::Preloading;

	if (RoomDefinition->PreloadPrimaryAssets.IsEmpty() == false)
	{
		UAssetManager& AssetManager = UR1AssetManager::Get();
		AssetManager.LoadPrimaryAssets(
			RoomDefinition->PreloadPrimaryAssets,
			TArray<FName>(),
			FStreamableDelegate::CreateWeakLambda(this, [this, RoomKey]()
			{
				if (FR1RoomRuntimeState* RuntimeState = RoomStates.Find(RoomKey))
				{
					RuntimeState->ThermalState = ER1RoomThermalState::Warm;
					RuntimeState->LastTouchedTime = FPlatformTime::Seconds();
				}
			}));
	}
	else
	{
		State.ThermalState = ER1RoomThermalState::Warm;
	}
}

void UR1RoomStreamingSubsystem::TrimPreloadIfNeeded()
{
	TArray<TPair<FName, FR1RoomRuntimeState*>> CachedRooms;
	for (auto& Pair : RoomStates)
	{
		if (Pair.Value.ThermalState != ER1RoomThermalState::Cold)
		{
			CachedRooms.Emplace(Pair.Key, &Pair.Value);
		}
	}

	CachedRooms.Sort([](const auto& A, const auto& B)
	{
		return A.Value->LastTouchedTime > B.Value->LastTouchedTime;
	});

	for (int32 Index = Budget.MaxPreloadedRooms; Index < CachedRooms.Num(); ++Index)
	{
		CachedRooms[Index].Value->ThermalState = ER1RoomThermalState::Cold;
		CachedRooms[Index].Value->StreamingLevel = nullptr;
	}
}

FName UR1RoomStreamingSubsystem::MakeRoomKey(const UR1RoomDefinitionData* RoomDefinition) const
{
	if (RoomDefinition == nullptr)
	{
		return NAME_None;
	}

	return FName(*RoomDefinition->GetPrimaryAssetId().ToString());
}

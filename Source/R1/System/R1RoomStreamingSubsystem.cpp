/**
 * UR1RoomStreamingSubsystem의 구현부입니다.
 * 비동기 에셋 로딩(Async Loading), 룸 상태 변경 알림, 캐시 유지 정책(TickRoomCachePolicy) 등을 
 * 실제로 처리하는 로직을 담고 있습니다.
 */

#include "System/R1RoomStreamingSubsystem.h"
#include "Data/R1RoomDefinitionData.h"
#include "Engine/LevelStreamingDynamic.h"
#include "System/R1AssetManager.h"
#include "Data/R1AssetData.h" // 만들어두신 헤더 추가
#include "Engine/StreamableManager.h"

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

ULevelStreamingDynamic* UR1RoomStreamingSubsystem::SpawnRoomLevel(UR1RoomDefinitionData* RoomDefinition, FVector Location, FRotator Rotation)
{
	if (!RoomDefinition || RoomDefinition->RoomLevel.IsNull()) return nullptr;

	const FName RoomKey = MakeRoomKey(RoomDefinition);
	FR1RoomRuntimeState& State = RoomStates.FindOrAdd(RoomKey);
	State.RoomDefinition = RoomDefinition;
	State.LastTouchedTime = FPlatformTime::Seconds();

	if (State.StreamingLevel) return State.StreamingLevel;

	bool bOutSuccess = false;
	State.StreamingLevel = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(
		this, RoomDefinition->RoomLevel, Location, Rotation, bOutSuccess
	);

	return State.StreamingLevel;
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
		if (State.ThermalState == ER1RoomThermalState::Cold) continue;

		// Hot(현재 플레이 중인 방)은 시간 갱신만 하고 절대 언로드하지 않음
		if (State.ThermalState == ER1RoomThermalState::Hot)
		{
			State.LastTouchedTime = Now;
			continue;
		}

		// 지정된 시간이 지나면 메모리에서 완전히 해제
		if (Now - State.LastTouchedTime > Budget.UnloadGraceSeconds)
		{
			UnloadRoomInternal(State);
		}
	}
	TrimPreloadIfNeeded();
}

void UR1RoomStreamingSubsystem::MarkRoomAsLeft(UR1RoomDefinitionData* RoomDefinition)
{

	if (RoomDefinition == nullptr) return;

	const FName RoomKey = MakeRoomKey(RoomDefinition);
	if (FR1RoomRuntimeState* State = RoomStates.Find(RoomKey))
	{
		// 방 상태가 Hot이었다면 Warm으로 낮추고, 
		// LastTouchedTime을 갱신하여 이때부터 'UnloadGraceSeconds(예: 5초)' 카운트다운을 시작하게 합니다!
		if (State->ThermalState == ER1RoomThermalState::Hot)
		{
			State->ThermalState = ER1RoomThermalState::Warm;
			State->LastTouchedTime = FPlatformTime::Seconds();
			UE_LOG(LogTemp, Warning, TEXT("[%s] 플레이어가 방을 떠났습니다. 쿨다운(Warm) 타이머를 시작합니다."), *RoomKey.ToString());
		}
	}
}

void UR1RoomStreamingSubsystem::UnloadAllRooms()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// 월드에 로드되어 있는 모든 스트리밍 레벨을 순회합니다.
	const TArray<ULevelStreaming*>& StreamingLevels = World->GetStreamingLevels();

	for (ULevelStreaming* Level : StreamingLevels)
	{
		// 동적으로 생성된 방(ULevelStreamingDynamic)들만 골라서 파괴 지시
		if (ULevelStreamingDynamic* DynamicLevel = Cast<ULevelStreamingDynamic>(Level))
		{
			DynamicLevel->SetShouldBeLoaded(false);
			DynamicLevel->SetShouldBeVisible(false);
			DynamicLevel->SetIsRequestingUnloadAndRemoval(true); // 메모리에서 완전히 파괴
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[RoomStreaming] 이전 층의 모든 방을 메모리에서 해제했습니다."));
}

void UR1RoomStreamingSubsystem::UnloadRoomInternal(FR1RoomRuntimeState& State)
{
	if (State.ThermalState == ER1RoomThermalState::Cold) return;

	if (State.StreamingLevel)
	{
		State.StreamingLevel->SetIsRequestingUnloadAndRemoval(true);
		State.StreamingLevel = nullptr;
	}

	if (State.RoomDefinition && !State.RoomDefinition->PreloadPrimaryAssets.IsEmpty())
	{
		UR1AssetManager::Get().UnloadPrimaryAssets(State.RoomDefinition->PreloadPrimaryAssets);
	}

	// [수정] 라벨을 통해 로드했던 에셋들의 핸들을 취소하고 메모리를 놓아줍니다.
	if (State.PreloadHandle.IsValid())
	{
		State.PreloadHandle->CancelHandle();
		State.PreloadHandle.Reset();
	}

	State.ThermalState = ER1RoomThermalState::Cold;
}

void UR1RoomStreamingSubsystem::BeginPreload(UR1RoomDefinitionData* RoomDefinition)
{
	if (!RoomDefinition) return;

	const FName RoomKey = MakeRoomKey(RoomDefinition);
	FR1RoomRuntimeState& State = RoomStates.FindOrAdd(RoomKey);

	State.RoomDefinition = RoomDefinition; // 포인터 저장
	State.LastTouchedTime = FPlatformTime::Seconds();

	if (State.ThermalState == ER1RoomThermalState::Hot || State.ThermalState == ER1RoomThermalState::Warm) return;

	State.ThermalState = ER1RoomThermalState::Preloading;

	// 1. 비동기 로딩할 에셋 경로들을 담을 배열
	TArray<FSoftObjectPath> PathsToLoad;

	if (!RoomDefinition->PreloadPrimaryAssets.IsEmpty())
	{
		for (const FPrimaryAssetId& AssetId : RoomDefinition->PreloadPrimaryAssets)
		{
			FSoftObjectPath Path = UR1AssetManager::Get().GetPrimaryAssetPath(AssetId);
			if (Path.IsValid()) PathsToLoad.AddUnique(Path);
		}
	}
	// [통합 2] 질문자님의 UR1AssetData 라벨(Label) 연동 방식!
	if (GlobalAssetData && !RoomDefinition->PreloadAssetLabels.IsEmpty())
	{
		for (const FName& Label : RoomDefinition->PreloadAssetLabels)
		{
			// 라벨로 묶인 에셋 세트를 가져옵니다.
			const FAssetSet& AssetSet = GlobalAssetData->GetAssetSetByLabel(Label);

			// 세트 안의 모든 에셋 경로를 로딩 목록에 추가합니다.
			for (const FAssetEntry& Entry : AssetSet.AssetEntries)
			{
				PathsToLoad.AddUnique(Entry.AssetPath);
			}
		}
	}

	if (PathsToLoad.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] 방의 에셋 선로딩을 시작합니다. (총 %d개)"), *RoomKey.ToString(), PathsToLoad.Num());

		// [수정] RequestAsyncLoad가 반환하는 핸들을 State.PreloadHandle에 저장합니다!
		State.PreloadHandle = UR1AssetManager::Get().GetStreamableManager().RequestAsyncLoad(
			PathsToLoad,
			FStreamableDelegate::CreateWeakLambda(this, [this, RoomKey]()
				{
					if (FR1RoomRuntimeState* RuntimeState = RoomStates.Find(RoomKey))
					{
						RuntimeState->ThermalState = ER1RoomThermalState::Warm;
						RuntimeState->LastTouchedTime = FPlatformTime::Seconds();
						UE_LOG(LogTemp, Warning, TEXT("[%s] 에셋 선로딩 완료! (Warm 상태 진입)"), *RoomKey.ToString());
					}
				})
		);
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
		// [수정] Hot(현재 플레이 중인 방)은 강제 삭제 대상에서 절대적으로 제외합니다.
		if (Pair.Value.ThermalState != ER1RoomThermalState::Cold &&
			Pair.Value.ThermalState != ER1RoomThermalState::Hot)
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
		// [수정] 단순 상태 변경이 아니라, 진짜로 메모리를 비우는 함수를 호출합니다!
		UnloadRoomInternal(*(CachedRooms[Index].Value));
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
